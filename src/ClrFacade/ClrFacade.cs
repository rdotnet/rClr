using RDotNet;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace Rclr
{
    /// <summary>
    /// The main access point for R and rClr code to interact with the Common Language Runtime
    /// </summary>
    /// <remarks>
    /// The purpose of this class is to host gather method written in C# in preference to C code in rClr.c.
    /// </remarks>
    public static class ClrFacade
    {
        /// <summary>
        /// Invoke an instance method of an object
        /// </summary>
        public static object CallInstanceMethod(object obj, string methodName, object[] arguments)
        {
            return InternalCallInstanceMethod(obj, methodName, true, arguments);
        }

        internal static object InternalCallInstanceMethod(object obj, string methodName, bool tryUseConverter, object[] arguments)
        {
            object result = null;
            try
            {
                LastCallException = string.Empty;

                arguments = ConvertSpecialObjects(arguments);

                Type[] types = getTypes(arguments);
                BindingFlags bf = BindingFlags.Public | BindingFlags.Instance | BindingFlags.InvokeMethod;
                var classType = obj.GetType();
                MethodInfo method = findMethod(classType, methodName, bf, types);
                if (method != null)
                {
                    // Reenable to address issue 15
                    // arguments = changeArgumentTypes(arguments, method);
                    result = invokeMethod(obj, arguments, method, tryUseConverter);
                }
                else
                    ThrowMissingMethod(classType, methodName, "instance", types);
            }
            catch (Exception ex)
            {
                if (!LogThroughR(ex))
                    throw;
            }
            return result;
        }

        /// <summary>
        /// Invoke a method on a type
        /// </summary>
        /// <param name="classType"></param>
        /// <param name="methodName"></param>
        /// <param name="arguments"></param>
        /// <returns></returns>
        public static object CallStaticMethod(Type classType, string methodName, object[] arguments)
        {
            return InternalCallStaticMethod(classType, methodName, true, arguments);
        }

        internal static object InternalCallStaticMethod(Type classType, string methodName, bool tryUseConverter, params object[] arguments)
        {
            if (arguments.GetType() == typeof(string[])) // workaround https://r2clr.codeplex.com/workitem/11
                arguments = new object[] { arguments };
            // In order to handle the R Date and POSIXt conversion, we have to standardise on UTC in the C layer. 
            // The CLR hosting API seems to only marshall to date-times to Unspecified (probably cannot do otherwise)
            // We need to make sure these are Utc DateTime at this point.
            arguments = ConvertSpecialObjects(arguments);
            Type[] types = getTypes(arguments);
            BindingFlags bf = BindingFlags.Public | BindingFlags.Static | BindingFlags.InvokeMethod;
            MethodInfo method = findMethod(classType, methodName, bf, types);
            if (method != null)
            {
                //if (method.GetParameters().Length == 1 && method.GetParameters()[0].ParameterType == typeof(object[]))
                //    arguments = new object[] { arguments }; // necessary for e.g. static void QueryTypes(params object[] blah)
                return invokeMethod(null, arguments, method, tryUseConverter);
            }
            else
                ThrowMissingMethod(classType, methodName, "static", types);
            return null;
        }

        internal static void ThrowMissingMethod(Type classType, string methodName, string modifier, Type[] types)
        {
            ReflectionHelper.ThrowMissingMethod(classType, methodName, modifier, types);     
        }
        
        /// <summary>
        /// Return a reference to the object currently handled by the custom data converter, if any is in use.
        /// </summary>
        /// <remarks>
        /// See https://rclr.codeplex.com/workitem/33
        /// </remarks>
        public static object CurrentObject 
        {
            get 
            {
                if (DataConverter == null) return null;
                return DataConverter.CurrentObject;
            }
        }

        public static SymbolicExpressionWrapper CreateSexpWrapperMs(long ptrValue)
        {
            return CreateSexpWrapper(new IntPtr(ptrValue));
        }

        public static SymbolicExpressionWrapper CreateSexpWrapper(IntPtr sexp)
        {
            if (sexp == IntPtr.Zero)
                throw new ArgumentNullException("ptrValue", "Pointer value is the null pointer");
            SymbolicExpression s = DataConverter.CreateSymbolicExpression(sexp);
            return new SymbolicExpressionWrapper(s);
        }

        /// <summary>
        /// Invokes a static method given the name of a type.
        /// </summary>
        public static object CallStaticMethod(string typename, string methodName, object[] arguments)
        {
            Type t = null;
            object result = null;
            try
            {
                LastCallException = string.Empty;
                t = GetType(typename);
                if (t == null)
                    throw new ArgumentException(String.Format("Type not found: {0}", typename));
                result = InternalCallStaticMethod(t, methodName, true, arguments);
            }
            catch (Exception ex)
            {
                if (!LogThroughR(ex))
                    throw;
            }
            return result;
        }

        /// <summary>
        /// Advanced debugging. A function to help diagnose issues at the C/.NET interface. 
        /// </summary>
        public static string DiagnoseMethodCall(object[] arguments)
        {
            var sb = new StringBuilder();
            try
            {
                LastCallException = string.Empty;
                for (int i = 0; i < arguments.Length; i++)
                {
                    var arg = arguments[i];
                    sb.Append(
                        arg == null ?
                        string.Format("Parameter {0} is a null reference", i) :
                        string.Format("Parameter {0} is not null and of type {1}", i, arg.GetType().FullName)
                        );
                    sb.Append(Environment.NewLine);
                }
            }
            catch (Exception ex)
            {
                var argEx = new ArgumentException("Method Parameter Diagnosis so far: " + sb.ToString(), ex);
                if (!LogThroughR(argEx))
                    throw argEx;
            }
            return sb.ToString();
        }

        // https://rclr.codeplex.com/workitem/15
        private static object[] changeArgumentTypes(object[] arguments, MethodInfo method)
        {
            var parameters = method.GetParameters();
            var hasFloatArrays = parameters.Any(p => p.ParameterType == typeof(float[]));
            if (!hasFloatArrays)
                return arguments;
            var result = (object[])arguments.Clone();
            for (int i = 0; i < Math.Min(arguments.Length, parameters.Length); i++)
            {
                if (parameters[i].ParameterType == typeof(float[]))
                {
                    var dblArray = arguments[i] as double[];
                    if (dblArray != null)
                        result[i] = Array.ConvertAll(dblArray, a => (float)a);
                    else
                        throw new NotSupportedException("Only the conversion from arrays of double to single precision is transparently supported");
                }
            }
            return result;
        }

        private static bool LogThroughR(Exception ex)
        {
            // Initially just wanted to print to R as below. HOWEVER
            // https://r2clr.codeplex.com/workitem/67
            // if (DataConverter != null)
            // {
            //     DataConverter.Error(FormatException(ex));
            //     return true;

            // }
            // Instead, using the following
            LastCallException = FormatExceptionInnermost(ex);
            LastException = LastCallException;
            // Rely on this returning false so that caller rethrows the exception, so that 
            // we can retrieve the error in the C layer in the MS.NET related code.
            return false;
        }

        private static string CheckSehExceptionAdditionalErrorMessage(Exception ex)
        {
            if (ex is System.Runtime.InteropServices.SEHException)
            {
                if (ErrorMessageProvider == null)
                    return "Caught an SEHException, but no additional information is available via ErrorMessageProvider";
                else
                    return ErrorMessageProvider();
            }
            else
                return null;
        }

        public static string FormatExceptionInnermost(Exception ex)
        {
            Exception innermost = ex;
            while (innermost.InnerException != null)
                innermost = innermost.InnerException;

            var additionalMsg = CheckSehExceptionAdditionalErrorMessage(innermost);

            var tle = innermost as ReflectionTypeLoadException; // https://rclr.codeplex.com/workitem/26
            StringBuilder sb = new StringBuilder();
            if (!string.IsNullOrEmpty(additionalMsg))
                sb.Append(FormatCustomMsg(additionalMsg));
            else
                sb.Append(FormatExceptionMessage(innermost));
            if (tle != null)
            {
                foreach (var e in tle.LoaderExceptions)
                    sb.Append(FormatExceptionMessage(e));
            }
            return sb.ToString();
        }

        private static string FormatCustomMsg(string additionalMsg)
        {
            var result = string.Format("External Error Message: {1}{0}", "\n", additionalMsg);
            return ToUnixNewline(result);
        }

        private static string ToUnixNewline(string result)
        {
           return result.Replace("\r\n", "\n");
        }

        private static string FormatExceptionMessage(Exception ex)
        {
            // Note that if using Environment.NewLine below instead of "\n", the rgui prompt is losing it
            // Actually even with the latter it is, but less so. Annoying.
            var result = string.Format("Type:    {1}{0}Message: {2}{0}Method:  {3}{0}Stack trace:{0}{4}{0}{0}",
                "\n", ex.GetType(), ex.Message, ex.TargetSite, ex.StackTrace);
            // See whether this helps with the Rgui prompt:
            return ToUnixNewline(result);
        }

        /// <summary>
        /// Gets/sets a data converter to customize or extend the marshalling of data between R and the CLR
        /// </summary>
        public static IDataConverter DataConverter { get; set; }

        /// <summary>
        /// Gets/sets a function delegate that can provide additional error message information. 
        /// </summary>
        /// <remarks>
        /// This is intended to cater for cases where exception caught is SEHException, 
        /// presumably because some native code called by .NET via P/Invoke failed. 
        /// The native code may provide a way to retrieve information; this property offers a path to retrieve this information.
        /// </remarks>
        public static Func<string> ErrorMessageProvider { get; set; }

        /// <summary>
        /// Gets if there is a custom data converter set on this facade
        /// </summary>
        public static bool DataConverterIsSet { get { return DataConverter != null; } }

        /// <summary>
        /// Creates an instance of an object, given the type name
        /// </summary>
        public static object CreateInstance(string typename, params object[] arguments)
        {
            object result = null;
            try
            {
                LastCallException = string.Empty;

                arguments = ConvertSpecialObjects(arguments);

                var t = GetType(typename);
                if (t == null)
                    throw new ArgumentException(string.Format("Could not determine Type from string '{0}'", typename));
                result = ((arguments == null || arguments.Length == 0)
                                  ? Activator.CreateInstance(t)
                                  : Activator.CreateInstance(t, arguments));
            }
            catch (Exception ex)
            {
                if (!LogThroughR(ex))
                    throw;
            }
            return result;
        }

        public static Type GetType(string typename)
        {
            if (string.IsNullOrEmpty(typename))
                throw new ArgumentException("missing type specification");

            var t = Type.GetType(typename);
            if (t == null)
            {
                var loadedAssemblies = System.AppDomain.CurrentDomain.GetAssemblies();
                var typeComponents = typename.Split(',');
                if (typeComponents.Length > 1) // "TheNamespace.TheShortTypeName,TheAssemblyName"
                {
                    string aName = typeComponents[typeComponents.Length - 1];
                    var assembly = loadedAssemblies.FirstOrDefault((x => x.GetName().Name == aName));
                    if (assembly == null)
                    {
                        Console.WriteLine(String.Format("Assembly not found: {0}", aName));
                        return null;
                    }
                    t = assembly.GetType(typeComponents[0]);
                }
                else // typeComponents.Length == 1
                {
                    // Then we only have something like "TheNamespace.TheShortTypeName", Need to parse all the assemblies.
                    string tName = typeComponents[0];
                    foreach (var item in loadedAssemblies)
                    {
                        var types = item.GetTypes();
                        t = types.FirstOrDefault((x => x.FullName == tName));
                        if ( t != null )
                            return t;
                    }
                }
                if (t == null)
                {
                    var msg = String.Format("Type not found: {0}", typename);
                    Console.WriteLine(msg);
                    return null;
                }
            }
            return t;
        }

        /// <summary>
        /// Gets the full name of a type.
        /// </summary>
        /// <remarks>For easier operations from the C code</remarks>
        public static string GetObjectTypeName(object obj)
        {
            var result = obj.GetType().FullName;
            return result;
        }

        /// <summary>
        /// Loads an assembly, using the Assembly.LoadFrom or Assembly.Load method depending on the argument
        /// </summary>
        public static Assembly LoadFrom(string pathOrAssemblyName)
        {
            Assembly result = null;
            if(File.Exists(pathOrAssemblyName))
                result = Assembly.LoadFrom(pathOrAssemblyName);
            else if (isFullyQualifiedAssemblyName(pathOrAssemblyName))
                result = Assembly.Load(pathOrAssemblyName);
            else
                // the use of LoadWithPartialName is deprecated, but this is highly convenient for the end user untill there is 
                // another safer and convenient alternative
#pragma warning disable 618, 612
                result = Assembly.LoadWithPartialName(pathOrAssemblyName);
#pragma warning restore 618, 612
            //Console.WriteLine(result.CodeBase);
            return result;
        }

        private static bool isFullyQualifiedAssemblyName(string p)
        {
            //"System.Windows.Presentation, Version=3.5.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089")            
            return p.Contains("PublicKeyToken=");
        }

        [Obsolete("This may be superseeded as this was swamping user with too long a list", false)]
        public static string[] GetMembers(object obj)
        {
            List<MemberInfo> members = new List<MemberInfo>();
            var classType = obj.GetType();
            members.AddRange(obj.GetType().GetMembers());
            var ifTypes = classType.GetInterfaces();
            for (int i = 0; i < ifTypes.Length; i++)
            {
                var t = ifTypes[i];
                members.AddRange(t.GetMembers());
            }
            var result = Array.ConvertAll(members.ToArray(), (x => x.Name));
            result = result.Distinct().ToArray();
            Array.Sort(result);
            return result;
        }

        /// <summary>
        /// Gets the numerical value of a Date in the R system. This is different from the origin of the CLI DateTime.
        /// </summary>
        public static double GetRDateDoubleRepresentation(DateTime date)
        {
            var tspan = GetUtcTimeSpanRorigin(ref date);
            var res = tspan.TotalDays;
            //            Console.WriteLine(res);
            return res;
        }

        private static TimeSpan GetUtcTimeSpanRorigin(ref DateTime date)
        {
            date = date.ToUniversalTime();
            //            Console.WriteLine("date: {0}", date); 
            var tspan = date - RDateOrigin; // POSIXct internal representation is always a linear scale with UTC origin RDateOrigin
            return tspan;
        }

        public static double GetRPosixCtDoubleRepresentation(DateTime date)
        {
            var tspan = GetUtcTimeSpanRorigin(ref date);
            var res = tspan.TotalSeconds;
            //            Console.WriteLine(res);
            return res;
        }

        /// <summary>
        /// Gets the numerical value of a Date in the R system. This is different from the origin of the CLI DateTime.
        /// </summary>
        public static double[] GetRDateDoubleRepresentations(DateTime[] dates)
        {
			var result = new double[dates.Length];
			for (int i = 0; i < dates.Length; i++) {
				result[i]=GetRDateDoubleRepresentation(dates[i]);
			}
            return result;
        }

        /// <summary>
        /// Gets the numerical value of a POSIXct in the R system. This is different from the origin of the CLI DateTime.
        /// </summary>
        public static double GetRDatePosixtcNumericValue(DateTime date)
        {
            // IMPORTANT: the default representation of POSIXct is local time (i.e. no time zone info)
            date = date.ToLocalTime();
            var dt = date - RDateOrigin;
            var res = dt.TotalSeconds;
            return res;
        }

        public static double[] GetRDatePosixtcNumericValues(DateTime[] dates)
        {
            var result = new double[dates.Length];
            for (int i = 0; i < dates.Length; i++)
            {
                result[i] = GetRDatePosixtcNumericValue(dates[i]);
            }
            return result;
        }

        public static DateTime[] DateTimeArrayToUtc(DateTime[] dateTimes)
        {
            var result = new DateTime[dateTimes.Length];
            for (int i = 0; i < result.Length; i++)
                result[i] = dateTimes[i].ToUniversalTime();
            return result;
        }

        /// <summary>
        /// Given the numerical representation of a date in R, return the equivalent CLI DateTime.
        /// </summary>
        /// <param name="rDateNumericValue">The numerical value in R, e.g. as.numeric(as.Date('2001-02-03'))</param>
        public static DateTime CreateDateFromREpoch(double rDateNumericValue)
        {
            var res = RDateOrigin + TimeSpan.FromDays(rDateNumericValue);
//            Console.WriteLine("dbl value: {0}", rDateNumericValue);
//            Console.WriteLine("dtime value: {0}", res.ToString());
            return res;
        }

        /// <summary>
        /// Given the numerical representation of a date in R, return the equivalent CLI DateTime.
        /// </summary>
        /// <param name="rDateNumericValue">The numerical value in R, e.g. as.numeric(as.POSIXct('2001-02-03'))</param>
        public static DateTime CreateDateFromRPOSIXct(double rDateNumericValue)
        {
            var res = RDateOrigin + TimeSpan.FromSeconds(rDateNumericValue);
//            Console.WriteLine("dbl value: {0}", rDateNumericValue);
//            Console.WriteLine("dtime value: {0}", res.ToString());
            return res;
        }

        /// <summary>
        /// Returns a string that represents the parameter passed.
        /// </summary>
        /// <remarks>This is useful e.g. to quickly check from R the CLR equivalent of an R POSIXt object</remarks>
        public static string ToString(object obj)
        {
            return obj.ToString();
        }

        public static DateTime[] CreateDateArrayFromREpoch(double[] rDateNumericValues)
        {
            return Array.ConvertAll(rDateNumericValues, CreateDateFromREpoch);
        }

		public static void SetFieldOrProperty (object obj, string name, object value)
		{
			if(obj == null) throw new ArgumentNullException();
			var b = BindingFlags.Public | BindingFlags.Instance;
			internalSetFieldOrProperty(obj.GetType(), name, b, obj, value);
		}

		public static void SetFieldOrProperty (Type type, string name, object value)
		{
			if(type == null) throw new ArgumentNullException();
			var b = BindingFlags.Public | BindingFlags.Static;
			internalSetFieldOrProperty(type, name, b, null, value);
		}

		public static void SetFieldOrProperty (string typename, string name, object value)
		{
            Type t = ClrFacade.GetType(typename);
			if (t == null)
                throw new ArgumentException(String.Format("Type not found: {0}", typename));
			SetFieldOrProperty(t, name, value);
		}

		static void internalSetFieldOrProperty (Type t, string name, BindingFlags b, object obj_or_null, object value)
		{
			var field = t.GetField (name, b);
			if (field == null) {
				var property = t.GetProperty (name, b);
				if (property == null)
					throw new ArgumentException (string.Format ("Public instance field or property name {0} not found", name));
				else
					property.SetValue (obj_or_null, value, null);
			}
			else
				field.SetValue (obj_or_null, value);
		}

        public static object GetFieldOrProperty (string typename, string name)
		{
            Type t = ClrFacade.GetType(typename);
			if (t == null)
                throw new ArgumentException(String.Format("Type not found: {0}", typename));
			return GetFieldOrPropertyType(t, name);
        }

        public static object GetFieldOrProperty (object obj, string name)
		{
            obj = ConvertSpecialObject(obj);
			var b = BindingFlags.Public | BindingFlags.Instance;
			Type t = obj.GetType ();
			return internalGetFieldOrProperty (t, name, b, obj);
		}

        private static object GetFieldOrPropertyType(Type type, string name)
		{
			var b = BindingFlags.Public | BindingFlags.Static;
			return internalGetFieldOrProperty (type, name, b, null);
		}

		static object internalGetFieldOrProperty (Type t, string name, BindingFlags b, object obj_or_null)
		{
			var field = t.GetField(name, b);
			if (field == null) {
				var property = t.GetProperty (name, b);
				if (property == null)
					throw new ArgumentException (string.Format ("Field or property name '{0}' not found on object of type '{1}', for binding flags '{2}'", name, t.Name, b.ToString()));
				else
					return property.GetValue (obj_or_null, null);
			}
			else
				return field.GetValue (obj_or_null);
		}
        /// <summary>
        /// A default binder for finding methods; a placeholder for a way to customize or refine the method selection process for rClr.
        /// </summary>
        private static Binder methodBinder = System.Type.DefaultBinder; // reverting; this causes problems for parameters with the params keyword

        private static MethodInfo findMethod(Type classType, string methodName, BindingFlags bf, Type[] types)
        {
            return ReflectionHelper.GetMethod(classType, methodName, methodBinder, bf, types);
        }

        private static object invokeMethod(object obj, object[] arguments, MethodInfo method, bool tryUseConverter)
        {
            var parameters = method.GetParameters();
            var numParameters = parameters.Length;
            if (numParameters > arguments.Length)
            {
                var newargs = new object[numParameters];
                arguments.CopyTo(newargs, 0);
                if (numParameters == (arguments.Length + 1))
                {
                    var lastParamInfo = parameters[parameters.Length-1];
                    if (ReflectionHelper.IsVarArg(lastParamInfo))
                        newargs[numParameters - 1] = Array.CreateInstance(lastParamInfo.ParameterType.GetElementType(), 0);
                }
                else
                {
                    // Assume this is because of parameters with default values, and handle as per:
                    // http://msdn.microsoft.com/en-us/library/x0acewhc.aspx
                    for (int i = arguments.Length; i < newargs.Length; i++)
                        newargs[i] = Type.Missing;
                }
                arguments = newargs;
            }
            else if (parameters.Length > 0)
            {
                // check whether we have a method with the last argument with a 'params' keyword
                // This is not handled magically when using reflection.
                var p = parameters[parameters.Length - 1];
                if (ReflectionHelper.IsVarArg(p))
                    arguments = packParameters(arguments, numParameters, p);
            }
            return marshallDataToR(method.Invoke(obj, arguments), tryUseConverter);
        }

        private static object[] packParameters(object[] arguments, int np, ParameterInfo p)
        {
            var arrayType = p.ParameterType;
            if (np < 1)
                throw new ArgumentException("numParameters must be strictly positive");
            if (!arrayType.IsArray)
                throw new ArgumentException("Inconsistent - arguments should not be packed with a non-array method parameter");
            return PackParameters(arguments, np, arrayType);
        }

        public static object[] PackParameters(object[] arguments, int np, Type arrayType)
        {
            // f(obj, string, params int[] integers) // numParameters = 3
            int na = arguments.Length;
            var tElement = arrayType.GetElementType(); // Int32 for an array int[]
            var result = new object[np];
            Array.Copy(arguments, result, np - 1); // obj, string
            if ((np == na) && (arrayType == arguments[na - 1].GetType()))
            {
                // we already have an int[] pre-packed. 
                // {obj, "methName", new int[]{p1, p2, p3})  length 3
                    // NOTE Possible singular and ambiguous cases: params object[] or params Array[]
                    Array.Copy(arguments, na - 1, result, na - 1, 1);
            }
            else
            {
                // {obj, "methName", p1, p2, p3)  length 5
                Array paramParam = Array.CreateInstance(tElement, na - np + 1); // na - np + 1 = 5 - 3 + 1 = 3
                Array.Copy(arguments, np - 1, paramParam, 0, na - np + 1); // np - 1 = 3 - 1 = 2 start index
                result.SetValue(paramParam, np - 1);
            }
            return result;
        }

        private static object marshallDataToR(object obj, bool tryUseConverter)
        {
            obj = conditionDateTime(obj);
            var result = (tryUseConverter && (DataConverter != null) ? DataConverter.ConvertToR(obj) : obj);
            if (result is SafeHandle)
                throw new NotSupportedException(string.Format("Object '{0}' is a SafeHandle and cannot be returned to native R", result.GetType().ToString()));
            return result;
        }

        private static object[] ConvertSpecialObjects(object[] arguments)
        {
            if (DataConverterIsSet)
                arguments = DataConverter.ConvertSymbolicExpressions(arguments);
            arguments = makeDatesUtcKind(arguments);
            return arguments;
        }

        private static object ConvertSpecialObject(object obj)
        {
            if (DataConverterIsSet)
                obj = DataConverter.ConvertSymbolicExpression(obj);
            if (obj is DateTime)
                obj = ForceUtcKind((DateTime)obj);
            return obj;
        }

        private static object[] makeDatesUtcKind(object[] arguments)
        {
            object[] newArgs = (object[])arguments.Clone();
            for (int i = 0; i < arguments.Length; i++)
            {
                var obj = arguments[i];
                if (obj is DateTime)
                    newArgs[i] = ForceUtcKind((DateTime)obj);
                else if (obj is DateTime[])
                    newArgs[i] = forceUtcKind((DateTime[])obj);
            }
            return newArgs;
        }

        private static DateTime[] forceUtcKind(DateTime[] dateTimes)
        {
            var result = new DateTime[dateTimes.Length];
            for (int i = 0; i < result.Length; i++)
            {
                result[i] = ForceUtcKind(dateTimes[i]);
            }
            return result;
        }

        public static DateTime ForceUtcKind(DateTime dateTime)
        {
            return ForceDateKind(dateTime, utc: true);
        }

        public static DateTime ForceDateKind(DateTime dateTime, bool utc = false)
        {
            return new DateTime(dateTime.Ticks, (utc ? DateTimeKind.Utc : DateTimeKind.Unspecified));
        }

        private static object conditionDateTime(object obj)
        {
            // 2013-05: move to have date-time in R as POSIXct objects. 
            // For reliability, only support UTC until specs are refined.
            // See unit tests in test-datetime.r
            if (obj == null) return obj;
            if (obj.GetType() == typeof(DateTime))
                return ((DateTime)obj).ToUniversalTime();
            else if (obj.GetType() == typeof(DateTime[]))
                return DateTimeArrayToUtc((DateTime[])obj);
            else
                return obj;
        }

        private static readonly DateTime RDateOrigin = new DateTime(1970,1,1);

        private static Type[] getTypes(object[] arguments)
        {
            // var result = Array.ConvertAll(arguments, (x => (x == null ? typeof(object) : x.GetType())));
            var result = new Type[arguments.Length];
            for (int i = 0; i < arguments.Length; i++) {
                result[i] = (arguments[i] == null ? typeof(object) : arguments[i].GetType ());
            }
            return result;
        }

        // Work around https://r2clr.codeplex.com/workitem/67

        /// <summary>
        /// A transient property with the printable format of the innermost exception of the latest clrCall[...] call.
        /// </summary>
        public static string LastCallException { get; private set; }

        /// <summary>
        /// A property with the printable format of the innermost exception of the last failed clrCall[...] call.
        /// </summary>
        public static string LastException { get; private set; }
    }
}
