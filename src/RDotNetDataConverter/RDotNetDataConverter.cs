using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using RDotNet;
using RDotNet.Internals;
using RDotNet.NativeLibrary;
using Rclr;
using System.Reflection;
using System.Numerics;

namespace Rclr
{
//    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
//    internal delegate void Rf_error(string msg);

    public class RDotNetDataConverter : IDataConverter
    {
        private RDotNetDataConverter (string pathToNativeSharedObj)
        {
            var dllName = pathToNativeSharedObj;
            // HACK - this feels wrong, at least not clean. All I have time for.
            if (string.IsNullOrEmpty(dllName))
            {
                string assmbPath = Assembly.GetAssembly(this.GetType()).Location;
                assmbPath = Path.GetFullPath(assmbPath);
                var libDir = Path.GetDirectoryName(assmbPath);
      
                if (NativeUtility.IsUnix)
                    dllName = Path.Combine(libDir, "rClrMono.so");
                else
                {
                    dllName = Path.Combine(libDir, Environment.Is64BitProcess ? "x64" : "i386", 
                        (isMonoRuntime() ? "rClrMono.dll" : "rClrMs.dll")
                    );
                }
            }

            DataConversionHelper.RclrNativeDll = new RclrUnmanagedDll(dllName);

            SetupREngine ();
            // The Mono API already has some unhandled exception reporting. 
            // TODO Use the following if it works well for both CLRuntimes.
#if !MONO
            SetupExceptionHandling();
#endif
            ConvertVectors = true;
            ConvertValueTypes = true;

            converterFunctions = new Dictionary<Type, Func<object, SymbolicExpression>>();

            converterFunctions.Add(typeof(float), ConvertSingle);
            converterFunctions.Add(typeof(double), ConvertDouble);
            converterFunctions.Add(typeof(byte), ConvertByte);
            converterFunctions.Add(typeof(bool), ConvertBool);
            converterFunctions.Add(typeof(int), ConvertInt);
            converterFunctions.Add(typeof(string), ConvertString);
            converterFunctions.Add(typeof(DateTime), ConvertDateTime);
            converterFunctions.Add(typeof(TimeSpan), ConvertTimeSpan);
            converterFunctions.Add(typeof(Complex), ConvertComplex);

            converterFunctions.Add(typeof(float[]), ConvertArraySingle);
            converterFunctions.Add(typeof(double[]), ConvertArrayDouble);
            converterFunctions.Add(typeof(byte[]), ConvertArrayByte);
            converterFunctions.Add(typeof(bool[]), ConvertArrayBool);
            converterFunctions.Add(typeof(int[]), ConvertArrayInt);
            converterFunctions.Add(typeof(string[]), ConvertArrayString);
            converterFunctions.Add(typeof(DateTime[]), ConvertArrayDateTime);
            converterFunctions.Add(typeof(Complex[]), ConvertArrayComplex);

            converterFunctions.Add(typeof(float[,]), ConvertMatrixSingle);
            converterFunctions.Add(typeof(double[,]), ConvertMatrixDouble);
            converterFunctions.Add(typeof(int[,]), ConvertMatrixInt);
            converterFunctions.Add(typeof(string[,]), ConvertMatrixString);

            converterFunctions.Add(typeof(float[][]), ConvertMatrixJaggedSingle);
            converterFunctions.Add(typeof(double[][]), ConvertMatrixJaggedDouble);
            converterFunctions.Add(typeof(int[][]), ConvertMatrixJaggedInt);
            converterFunctions.Add(typeof(string[][]), ConvertMatrixJaggedString);

            converterFunctions.Add(typeof(Dictionary<string, double>), ConvertDictionary<double>);
            converterFunctions.Add(typeof(Dictionary<string, float>), ConvertDictionary<float>);
            converterFunctions.Add(typeof(Dictionary<string, string>), ConvertDictionary<string>);
            converterFunctions.Add(typeof(Dictionary<string, int>), ConvertDictionary<int>);
            converterFunctions.Add(typeof(Dictionary<string, DateTime>), ConvertDictionary<DateTime>);

            converterFunctions.Add(typeof(Dictionary<string, double[]>), ConvertDictionary<double[]>);
            converterFunctions.Add(typeof(Dictionary<string, float[]>), ConvertDictionary<float[]>);
            converterFunctions.Add(typeof(Dictionary<string, string[]>), ConvertDictionary<string[]>);
            converterFunctions.Add(typeof(Dictionary<string, int[]>), ConvertDictionary<int[]>);
            converterFunctions.Add(typeof(Dictionary<string, DateTime[]>), ConvertDictionary<DateTime[]>);

            // Add some default converters for more general types
            converterFunctions.Add(typeof(Array), ConvertArrayObject);
            converterFunctions.Add(typeof(object), ConvertObject);

        }

        private bool isMonoRuntime()
        {
            return ClrFacade.IsMonoRuntime;
        }

        private void SetupExceptionHandling()
        {
            AppDomain.CurrentDomain.UnhandledException += OnUnhandleException;
        }

        private void OnUnhandleException(object sender, UnhandledExceptionEventArgs e)
        {
            Exception ex = (Exception)e.ExceptionObject;
            Error(ClrFacade.FormatExceptionInnermost(ex));
        }

        public void Error(string msg)
        {
         // TODO consider removing; since this looked like not working.
            throw new NotSupportedException();
            //engine.Error(msg);
        }

        public object CurrentObject { get { return CurrentObjectToConvert; } }

        private static void SetUseRDotNet(bool useIt)
        {
            IntPtr UseRDotNet = DataConversionHelper.RclrNativeDll.GetFunctionAddress("use_rdotnet");
            Marshal.WriteInt32(UseRDotNet, useIt ? 1 : 0);
        }

        /// <summary>
        /// Enable/disable the use of this data converter in the R-CLR interop data marshalling.
        /// </summary>
        public static void SetRDotNet(bool setit, string pathToNativeSharedObj = null)
        {
            if (setit)
                ClrFacade.DataConverter = GetInstance(pathToNativeSharedObj);
            else
                ClrFacade.DataConverter = null;
            SetUseRDotNet(setit);
        }

        /// <summary>
        /// Convert an object, if possible, using RDotNet capabilities
        /// </summary>
        /// <remarks>
        /// If a conversion to an RDotNet SymbolicExpression was possible, 
        /// this returns the IntPtr SafeHandle.DangerousGetHandle() to be passed to R.
        /// If the object is null or such that no known conversion is possible, the same object 
        /// as the input parameter is returned.
        /// </remarks>
        public object ConvertToR (object obj)
        {
            ClearSexpHandles();
            if (obj == null) 
                return null;
            var sexp = obj as SymbolicExpression;
            if (sexp != null)
                return ReturnHandle(sexp);

            sexp = TryConvertToSexp(obj);

            if (sexp == null)
                return obj;
            return ReturnHandle(sexp);
        }

        private void ClearSexpHandles()
        {
            handles.Clear();
        }

        private static object ReturnHandle(SymbolicExpression sexp)
        {
            AddSexpHandle(sexp);
            return sexp.DangerousGetHandle();
        }

        private static void AddSexpHandle(SymbolicExpression sexp)
        {
            handles.Add(sexp);
        }

        /// <summary>
        /// A list to reference to otherwise transient SEXP created by this class. 
        /// This is to prevent .NET and R to trigger GC before rClr function calls have returned to R.
        /// </summary>
        private static List<SymbolicExpression> handles = new List<SymbolicExpression>();

        public object ConvertFromR(IntPtr pointer, int sexptype)
        {
            throw new NotImplementedException();
            //return new DataFrame(engine, pointer);
        }

        public bool ConvertVectors { get; set; }
        public bool ConvertValueTypes { get; set; }

        private void SetupREngine()
        {
            engine = REngine.GetInstance(initialize: false);
            engine.Initialize(setupMainLoop: false);
        }

        private static RDotNetDataConverter singleton;

        private static RDotNetDataConverter GetInstance(string pathToNativeSharedObj)
        {
            // Make sure this is set only once (RDotNet known limitation to one engine per session, effectively a singleton).
            if (singleton == null)
                singleton = new RDotNetDataConverter(pathToNativeSharedObj);
            return singleton;
        }

        private Dictionary<Type, Func<object, SymbolicExpression>> converterFunctions;

        private SymbolicExpression TryConvertToSexp(object obj)
        {
            SymbolicExpression sHandle = null;
            if (obj == null)
                throw new ArgumentNullException("object to convert to R must not be a null reference");
            var converter = TryGetConverter(obj);
            sHandle = (converter == null ? null : converter.Invoke(obj));
            return sHandle;
        }

        private Func<object, SymbolicExpression> TryGetConverter(object obj)
        {
            var t = obj.GetType();
            Func<object, SymbolicExpression> converter;
            if (converterFunctions.TryGetValue(t, out converter))
                return converter;
            if (TryGetGenericConverters(obj, out converter))
                return converter;
            return null;
        }

        private Func<object, SymbolicExpression> TryGetConverter(Type t)
        {
            Func<object, SymbolicExpression> converter;
            if (converterFunctions.TryGetValue(t, out converter))
                return converter;
            return null;
        }

        private bool TryGetGenericConverters(object obj, out Func<object, SymbolicExpression> converter)
        {
            var t = obj.GetType();
            if (typeof(Array).IsAssignableFrom(t))
            {
                Array a = obj as Array;
                if (a.Rank == 1)
                    return (converterFunctions.TryGetValue(typeof(Array), out converter));
            }
            converter = null;
            return (converterFunctions.TryGetValue(typeof(object), out converter));
        }

        //private bool TryGetValueAssignableValue(Type t, out Func<object, SymbolicExpression> converter)
        //{
        //    var assignable = converterFunctions.Keys.Where(x => x.IsAssignableFrom(t)).FirstOrDefault();
        //    if(assignable!=null)
        //    {
        //        assignable.
        //    if(converterFunctions.TryGetValue(t, out converter)
        //}

        private SymbolicExpression ConvertToSexp(object obj)
        {
            if (obj == null) return null;
            var result = TryConvertToSexp(obj);
            if(result==null)
                throw new NotSupportedException(string.Format("Cannot yet expose type {0} as a SEXP", obj.GetType().FullName));
            return result;
        }

        private GenericVector ConvertDictionary<U>(object obj)
        {
            var dict = (IDictionary<string, U>)obj;
            if (!converterFunctions.ContainsKey(typeof(U[])))
                throw new NotSupportedException("Cannot convert a dictionary of type " + dict.GetType()); 
            var values = converterFunctions[typeof(U[])].Invoke(dict.Values.ToArray());
            SetAttribute(values, dict.Keys.ToArray());
            return values.AsList();
        }

        private SymbolicExpression ConvertAll(object[] objects, Func<object, SymbolicExpression> converter=null)
        {
            var sexpArray = new SymbolicExpression[objects.Length];
            for (int i = 0; i < objects.Length; i++)
                sexpArray[i] = converter == null ? ConvertToSexp(objects[i]) : converter(objects[i]);
            return new GenericVector(engine, sexpArray);
        }

        private SymbolicExpression ConvertArrayDouble(object obj)
        {
            if (!ConvertVectors) return null;
            double[] array = (double[])obj;
            return engine.CreateNumericVector(array);
        }

        private SymbolicExpression ConvertArrayBool(object obj)
        {
            if (!ConvertVectors) return null;
            bool[] array = (bool[])obj;
            return engine.CreateLogicalVector(array);
        }

        private SymbolicExpression ConvertArrayByte(object obj)
        {
            if (!ConvertVectors) return null;
            byte[] array = (byte[])obj;
            return engine.CreateRawVector(array);
        }

        private SymbolicExpression ConvertArraySingle(object obj)
        {
            if (!ConvertVectors) return null;
            float[] array = (float[])obj;
            return ConvertArrayDouble(Array.ConvertAll(array, x => (double)x));
        }

        private SymbolicExpression ConvertArrayInt(object obj)
        {
            if (!ConvertVectors) return null;
            int[] array = (int[])obj;
            return engine.CreateIntegerVector(array);
        }

        private SymbolicExpression ConvertArrayString(object obj)
        {
            if (!ConvertVectors) return null;
            string[] array = (string[])obj;
            return engine.CreateCharacterVector(array);
        }

        private SymbolicExpression ConvertArrayDateTime(object obj)
        {
            if (!ConvertVectors) return null;
            if (!ConvertValueTypes) return null;
            DateTime[] array = (DateTime[])obj;
            var doubleArray = Array.ConvertAll(array, ClrFacade.GetRPosixCtDoubleRepresentation);
            var result = ConvertArrayDouble(doubleArray);
            SetClassAttribute(result, "POSIXct", "POSIXt");
            SetTzoneAttribute(result, "UTC");
            return result;
        }

        private SymbolicExpression ConvertArrayComplex(object obj)
        {
            if (!ConvertVectors) return null;
            if (!ConvertValueTypes) return null;
            Complex[] array = (Complex[])obj;
            return engine.CreateComplexVector(array);
        }        

        private SymbolicExpression ConvertArrayTimeSpan(object obj)
        {
            if (!ConvertVectors) return null;
            if (!ConvertValueTypes) return null;
            TimeSpan[] array = (TimeSpan[])obj;
            var doubleArray = Array.ConvertAll(array, (x => x.TotalSeconds));
            var result = ConvertArrayDouble(doubleArray);
            SetClassAttribute(result, "difftime"); // class(as.difftime(3.5, units='secs'))
            SetUnitsAttribute(result, "secs");  // unclass(as.difftime(3.5, units='secs'))
            return result;
        }

        private SymbolicExpression ConvertDouble(object obj)
        {
            if (!ConvertVectors) return null;
            double value = (double)obj;
            return engine.CreateNumeric(value);
        }

        private SymbolicExpression ConvertSingle(object obj)
        {
            if (!ConvertVectors) return null;
            float value = (float)obj;
            return ConvertArrayDouble((double)value);
        }

        private SymbolicExpression ConvertByte(object obj)
        {
            if (!ConvertVectors) return null;
            byte value = (byte)obj;
            return engine.CreateRaw(value);
        }

        private SymbolicExpression ConvertBool(object obj)
        {
            if (!ConvertVectors) return null;
            bool value = (bool)obj;
            return engine.CreateLogical(value);
        }

        private SymbolicExpression ConvertInt(object obj)
        {
            if (!ConvertVectors) return null;
            int value = (int)obj;
            return engine.CreateInteger(value);
        }

        private SymbolicExpression ConvertString(object obj)
        {
            if (!ConvertVectors) return null;
            string value = (string)obj;
            return engine.CreateCharacter(value);
        }

        private SymbolicExpression ConvertDateTime(object obj)
        {
            if (!ConvertVectors) return null;
            if (!ConvertValueTypes) return null;
            DateTime value = (DateTime)obj;
            var doubleValue = ClrFacade.GetRPosixCtDoubleRepresentation(value);
            var result = ConvertDouble(doubleValue);
            SetClassAttribute(result, "POSIXct", "POSIXt");
            SetTzoneAttribute(result, "UTC");
            return result;
        }

        private SymbolicExpression ConvertComplex(object obj)
        {
            if (!ConvertVectors) return null;
            if (!ConvertValueTypes) return null;
            Complex value = (Complex)obj;
            return engine.CreateComplex(value);
        }

        private SymbolicExpression ConvertTimeSpan(object obj)
        {
            if (!ConvertVectors) return null;
            if (!ConvertValueTypes) return null;
            TimeSpan value = (TimeSpan)obj;
            var doubleValue = value.TotalSeconds;
            var result = ConvertDouble(doubleValue);
            SetClassAttribute(result, "difftime"); // class(as.difftime(3.5, units='secs'))
            SetUnitsAttribute(result, "secs");  // unclass(as.difftime(3.5, units='secs'))
            return result;
        }

        private SymbolicExpression ConvertArrayObject(object obj)
        {
            Array a = (Array) obj;
            return ConvertToList(a);
        }

        private SymbolicExpression ConvertObject(object obj)
        {
            if (obj == null)
                return engine.NilValue;
            return CreateClrObj(obj);
            //var ptr = DataConversionHelper.ClrObjectToSexp(obj);
            //if (ptr == IntPtr.Zero)
            //    return null; // we did not manage to convert here. Fallback on native layer of rClr used later on.
            //var externalPtr = new ExternalPointer(engine, ptr);
            // At this point, we have a loop from managed to unmanaged to managed memory.
            //  ExternalPointer -> externalptr -> ClrObjectHandle -> Variant(if Microsoft) -> obj
            // This is not quite what we want to return: we need to produce an S4 object 
            //  S4Object -> ExternalPointer -> externalptr -> ClrObjectHandle -> Variant(if Microsoft) -> obj
            // return CreateClrObj(externalPtr, obj.GetType().FullName);
        }

        private Function createClrS4Object;

        public Function CreateClrS4Object_obsolete
        {
            get
            {
                if (createClrS4Object == null)
                    createClrS4Object = engine.Evaluate("invisible(function(objExtPtr, typename) { new('cobjRef', clrobj=objExtPtr, clrtype=typename) })").AsFunction();
                return createClrS4Object;
            }
        }

        public Function CreateClrS4Object
        {
            get
            {
                if (createClrS4Object == null)
                    createClrS4Object = engine.Evaluate("invisible(getCurrentConvertedObject)").AsFunction();
                return createClrS4Object;
            }
        }

        private S4Object CreateClrObj_obsolete(ExternalPointer ptr, string typename)
        {
            return CreateClrS4Object.Invoke(ptr, engine.CreateCharacter(typename)).AsS4();
        }

        public static object CurrentObjectToConvert { get; private set; }

        private S4Object CreateClrObj(object obj)
        {
            CurrentObjectToConvert = obj;
            var result = CreateClrS4Object.Invoke().AsS4();
            CurrentObjectToConvert = null;
            return result;
        }

        private class ClrObjectWrapper : S4Object
        {
            public ClrObjectWrapper(REngine engine, IntPtr pointer)
                : base(engine, pointer)
            {
            }
        }

        private class ExternalPointer : SymbolicExpression
        {
            public ExternalPointer(REngine engine, IntPtr pointer)
                : base(engine, pointer)
            {
            }
        }

        private SymbolicExpression ConvertToList(Array a)
        {
            if (a.Rank > 1)
                throw new NotSupportedException("Generic array converter is limited to uni-dimensional arrays");
            // CAUTION: The following, while efficient, means that mroe specialised converters
            // will not be picked up.
            var elementConverter = TryGetConverter(a.GetType().GetElementType());
            object[] tmp = new object[a.GetLength(0)];
            Array.Copy(a, tmp, tmp.Length);
            return ConvertAll(tmp, elementConverter);
        }

        private SymbolicExpression ConvertMatrixJaggedSingle(object obj)
        {
            float[][] array = (float[][])obj;
            if (array.IsRectangular())
                return ConvertMatrixDouble(array.ToDoubleRect());
            else
                return ConvertToList(array.ToDouble());
        }

        private SymbolicExpression ConvertMatrixJaggedDouble(object obj)
        {
            double[][] array = (double[][])obj;
            if (array.IsRectangular())
                return ConvertMatrixDouble(array.ToRect());
            else
                return ConvertToList(array);
        }

        private SymbolicExpression ConvertMatrixJaggedInt(object obj)
        {
            int[][] array = (int[][])obj;
            if (array.IsRectangular())
                return ConvertMatrixInt(array.ToRect());
            else
                return ConvertToList(array);
        }

        private SymbolicExpression ConvertMatrixJaggedString(object obj)
        {
            string[][] array = (string[][])obj;
            if (array.IsRectangular())
                return ConvertMatrixString(array.ToRect());
            else
                return ConvertToList(array);
        }

        private NumericMatrix ConvertMatrixSingle(object obj)
        {
            float[,] array = (float[,])obj;
            return ConvertMatrixDouble(array.ToDoubleRect());
        }

        private NumericMatrix ConvertMatrixDouble(object obj)
        {
            double[,] array = (double[,])obj;
            return engine.CreateNumericMatrix(array);
        }

        private IntegerMatrix ConvertMatrixInt(object obj)
        {
            int[,] array = (int[,])obj;
            return engine.CreateIntegerMatrix(array);
        }

        private CharacterMatrix ConvertMatrixString(object obj)
        {
            string[,] array = (string[,])obj;
            return engine.CreateCharacterMatrix(array);
        }

        private void SetTzoneAttribute(SymbolicExpression sexp, string tzoneId)
        {
            SetAttribute(sexp, new[] { tzoneId }, attributeName: "tzone");
        }

        private void SetUnitsAttribute(SymbolicExpression sexp, string units)
        {
            SetAttribute(sexp, new[] { units }, attributeName: "units");
        }

        private void SetClassAttribute(SymbolicExpression sexp, params string[] classes)
        {
            SetAttribute(sexp, classes, attributeName: "class");
        }

        private void SetAttribute(SymbolicExpression sexp, string[] attribValues, string attributeName = "names")
        {
            var names = new CharacterVector(engine, attribValues);
            sexp.SetAttribute(attributeName, names);
        }

        [Obsolete()]
        private static void CheckEnvironmentVariables ()
        {
            var rlibFilename = getRDllName ();
            var searchPaths = (Environment.GetEnvironmentVariable ("PATH") ?? "").Split (Path.PathSeparator);
//            if( !searchPaths.Contains("/usr/lib"))
//                searchPaths.ToList().Add()
            var pathsWithRdll = searchPaths.Where ((x => File.Exists (Path.Combine (x, rlibFilename))));
            bool rdllInPath = (pathsWithRdll.Count () > 0);
            if (!rdllInPath)
                throw new Exception (string.Format("'{0}' not found in any of the paths in environment variable PATH", rlibFilename));
            var rhome = (Environment.GetEnvironmentVariable ("R_HOME") ?? "");
            if (string.IsNullOrEmpty (rhome)) {
                // It is OK: the call to Initialize on the REngine will set up R_HOME.
                //throw new Exception("environment variable R_HOME is not set");
            }
        }

        private static string getRDllName ()
        {
            return NativeUtility.GetRLibraryFileName();
        }

        private REngine engine;

        public REngine GetEngine()
        {
            return engine;
        }

        public SymbolicExpression CreateSymbolicExpression(IntPtr sexp)
        {
            return engine.CreateFromNativeSexp(sexp);
        }

        public object[] ConvertSymbolicExpressions(object[] arguments)
        {
            object[] result = (object[])arguments.Clone();
            for (int i = 0; i < result.Length; i++)
            {
                result[i] = ConvertSymbolicExpression(arguments[i]);
            }
            return result;
        }

        public object ConvertSymbolicExpression(object obj)
        {
            if (obj is SymbolicExpressionWrapper)
                return ConvertSymbolicExpression(obj as SymbolicExpressionWrapper);
            else
                return obj;
        }

        private object ConvertSymbolicExpression(SymbolicExpressionWrapper sexpWrap)
        {
            return sexpWrap.ToClrEquivalent();
        }
    }
}