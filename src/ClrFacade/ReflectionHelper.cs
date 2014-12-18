using Rclr.Tests;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;

namespace Rclr
{
    /// <summary>
    /// Gathers the reflection operations on objects performed by rClr, as well as discovery operations on the CLR.
    /// </summary>
    public static class ReflectionHelper
    {
        /// <summary>
        /// Gets information on the common language runtime on which this code is executing. 
        /// Purpose is to have human-readable information ot diagnose interop issues between R and the CLR runtime.
        /// </summary>
        public static string[] GetClrInfo()
        {
            var result = new List<string>();
            result.Add(System.Environment.Version.ToString());
            return result.ToArray();
        }

        /// <summary>
        /// Gets the simple names of assemblies loaded in the current domain.
        /// </summary>
        public static string[] GetLoadedAssemblyNames(bool fullName=false)
        {
            var loadedAssemblies = AppDomain.CurrentDomain.GetAssemblies();
            return Array.ConvertAll(loadedAssemblies, x => (fullName ? x.GetName().FullName : x.GetName().Name));
        }

        /// <summary>
        /// Gets the paths of assemblies if loaded in the current domain.
        /// </summary>
        public static string[] GetLoadedAssemblyURI(string[] assemblyNames)
        {
            var loadedAssemblies = AppDomain.CurrentDomain.GetAssemblies();
            string[] result = new string[assemblyNames.Length];
            for (int i = 0; i < assemblyNames.Length; i++)
            {
                var s = assemblyNames[i];
                var a = loadedAssemblies.First(x => matchAssemblyName(x, s));
                result[i] = a == null ? "<not found>" : a.EscapedCodeBase; 
            }
            return result;
        }

        private static bool matchAssemblyName(Assembly a, string name)
        {
            var an = a.GetName();
            return an.FullName == name || an.Name == name;
        }

        /// <summary>
        /// Gets the full name of types (Type.FullName) contained in an assembly, given its simple name
        /// </summary>
        public static string[] GetTypesInAssembly(string assemblyName)
        {
            var loadedAssemblies = AppDomain.CurrentDomain.GetAssemblies();
            var assembly = loadedAssemblies.FirstOrDefault((x => x.GetName().Name == assemblyName));
            if (assembly == null)
                return new []{string.Format("Assembly '{0}' not found", assemblyName)};
            var types = assembly.GetExportedTypes();
            return Array.ConvertAll(types, t => t.FullName);
        }

        /// <summary>
        /// Gets human-readable signatures of the member(s) of an object or its type. 
        /// The purpose is to explore CLR object members from R.
        /// </summary>
        /// <param name="obj">The object to reflect on, or the type of the object if already known</param>
        /// <param name="memberName">The name of the objec/class member, e.g. the method name</param>
        public static string[] GetSignature(object obj, string memberName)
        {
            //if(obj==null)
            //    return new string[0];
            Type type = obj as Type;
            if (type != null)
                return GetSignature_Type(type, memberName);
            else
                return GetSignature_Type(obj.GetType(), memberName);
        }

        public static string[] GetSignature(string typeName, string memberName)
        {
            Type type = ClrFacade.GetType(typeName);
            if (type != null)
                return GetSignature_Type(type, memberName);
            else
                return new string[]{};
        }

        /// <summary>
        /// Gets human-readable signatures of the member(s) of a type. 
        /// </summary>
        /// <param name="obj">The type to reflect on</param>
        /// <param name="memberName">The name of the objec/class member, e.g. the method name</param>
        /// <param name="type"></param>
        /// <returns></returns>
        public static string[] GetSignature_Type(Type type, string memberName)
        {
            var members = type.GetMember(memberName);
            string[] result = summarize(members);
            return result;
        }

        /// <summary>
        /// Finds the first method in a type that matches a method name. 
        /// Explicit interface implementations are searched if required.
        /// </summary>
        /// <param name="classType"></param>
        /// <param name="methodName"></param>
        /// <param name="binder"></param>
        /// <param name="bf"></param>
        /// <param name="types"></param>
        /// <returns></returns>
        public static MethodInfo GetMethod(Type classType, string methodName, Binder binder, BindingFlags bf, Type[] types)
        {
            var method = classType.GetMethod(methodName, bf, binder, types, null);
            if (method == null)
            {
                var ifTypes = classType.GetInterfaces();
                for (int i = 0; i < ifTypes.Length; i++)
                {
                    var t = ifTypes[i];
                    method = t.GetMethod(methodName, bf, binder, types, null);
                    if (method != null)
                        return method;
                }
            }
            if (method == null)
                method = findDefaultParameterMethod(classType, methodName, bf, binder, types);
            if (method == null)
                method = findVarargMethod(classType, methodName, bf, binder, types); ;
            return method;
        }

        private static MethodInfo findDefaultParameterMethod(Type classType, string methodName, BindingFlags bf, Binder binder, Type[] types)
        {
            var methods = classType.GetMethods(bf).Where(m => m.Name == methodName).Where(m => HasOptionalParams(m));
            if (methods.Count() == 0) return null;
            var mi = methods.Where(m => ExactTypeMatchesOptionalParams(m, types));
            if (mi.Count() == 0)
                mi = methods.Where(m => AssignableTypesMatchesOptionalParams(m, types));
            if (mi.Count() > 1)
                mi = GetLowestParameterMatch(mi, types, GetFirstExactMatchOptionalParams);
            if (mi.Count() > 1)
                throwAmbiguousMatch(mi);
            return mi.FirstOrDefault();
        }

        private static int Min(IEnumerable<int> indices) { return indices.Min(); }
        private static int Max(IEnumerable<int> indices) { return indices.Max(); } 

        private static IEnumerable<MethodInfo> GetLowestParameterMatch(IEnumerable<MethodInfo> mi, Type[] types, Func<MethodInfo, Type[], int> indexTest)
        {
            return GetBestParameterMatch(mi, types, indexTest, Min);
        }

        private static IEnumerable<MethodInfo> GetHighestParameterMatch(IEnumerable<MethodInfo> mi, Type[] types, Func<MethodInfo, Type[], int> indexTest)
        {
            return GetBestParameterMatch(mi, types, indexTest, Max);
        }

        private static IEnumerable<MethodInfo> GetBestParameterMatch(IEnumerable<MethodInfo> mi, Type[] types, Func<MethodInfo, Type[], int> indexTest, Func<IEnumerable<int>, int> bestScore)
        {
            var candidates = mi.ToList();
            var indicesFirstMatch = GetIndexFirstMatch(mi, types, indexTest);
            var validIndices = GetPositivesOnly(indicesFirstMatch);
            if (validIndices.Count() == 0) return new MethodInfo[0];
            var result = new List<MethodInfo>();
            var bestIndex = bestScore(validIndices);
            for (int i = 0; i < candidates.Count(); i++)
            {
                if (indicesFirstMatch[i] == bestIndex)
                    result.Add(candidates[i]);
            }
            return result;
        }

        private static IEnumerable<int> GetPositivesOnly(List<int> indicesFirstMatch)
        {
            var validIndices = indicesFirstMatch.Where(x => x >= 0);
            return validIndices;
        }

        private static List<int> GetIndexFirstMatch(IEnumerable<MethodInfo> mi, Type[] types, Func<MethodInfo, Type[], int> indexTest)
        {
            var indicesFirstMatch = mi.Select(m => indexTest(m, types)).ToList();
            return indicesFirstMatch;
        }

        private static void throwAmbiguousMatch(IEnumerable<MethodInfo> mi)
        {
            var s = Environment.NewLine + TestUtilities.Concat(mi.Select(x => x.ToString()), Environment.NewLine) + Environment.NewLine;
            throw new AmbiguousMatchException(s);
        }

        private static MethodInfo findVarargMethod(Type classType, string methodName, BindingFlags bf, Binder binder, Type[] types)
        {
            var methods = classType.GetMethods(bf).Where(m => m.Name==methodName).Where(m => HasVarArgs(m));
            if (methods.Count() == 0) return null;
            var mi = methods.Where(m => ExactTypeMatchesVarArgs(m, types));
            if (mi.Count() == 0)
                mi = methods.Where(m => AssignableTypesMatchesVarArgs(m, types));
            if (mi.Count() == 1)
                return mi.FirstOrDefault();
            else if (mi.Count() > 1)
            {
                // Try to see whether an exact match on the the non-params parameters, or on 
                // the params parameters, removes the ambiguation.
                var desambiguation = mi.Where(m => PartialExactTypesMatchesVarArgs(m, types));
                if (desambiguation.Count() == 1)
                    return desambiguation.FirstOrDefault();
                desambiguation = mi.Where(m => VarArgsExactTypesMatchesVarArgs(m, types));
                if (desambiguation.Count() == 1)
                    return desambiguation.FirstOrDefault();
                else // last resort
                {
                    var closestMatches = GetLowestParameterMatch(mi, types, GetFirstExactMatchVarargMethods);
                    if (closestMatches.Count() <= 1)
                        return closestMatches.FirstOrDefault();
                    closestMatches = GetHighestParameterMatch(closestMatches, types, GetLastExactMatchVarargMethods); // See https://rclr.codeplex.com/workitem/30
                    if (closestMatches.Count() <= 1)
                        return closestMatches.FirstOrDefault();
                    else
                        // too hard basket. For now. 
                        throwAmbiguousMatch(closestMatches);
                }
            }
            // nothing found
            return null;
        }

        private static bool ExactTypeMatchesOptionalParams(MethodInfo method, Type[] types)
        {
            return TestTypeMatchesOptionalParams(method, types, equals);
        }

        private static bool AssignableTypesMatchesOptionalParams(MethodInfo method, Type[] types)
        {
            return TestTypeMatchesOptionalParams(method, types, isAssignable);
        }

        private static bool ExactTypeMatchesVarArgs(MethodInfo method, Type[] types)
        {
            return TestTypeMatchesVarArgs(method, types, equals, equals);
        }

        private static bool PartialExactTypesMatchesVarArgs(MethodInfo method, Type[] types)
        {
            return TestTypeMatchesVarArgs(method, types, equals, isAssignable);
        }

        private static bool VarArgsExactTypesMatchesVarArgs(MethodInfo method, Type[] types)
        {
            return TestTypeMatchesVarArgs(method, types, isAssignable, equals);
        }

        private static bool AssignableTypesMatchesVarArgs(MethodInfo method, Type[] types)
        {
            return TestTypeMatchesVarArgs(method, types, isAssignable, isAssignable);
        }

        private static bool equals(Type methodType, Type paramType)
        {
            return methodType.Equals(paramType);
        }

        private static bool isAssignable(Type methodType, Type paramType)
        {
            return methodType.IsAssignableFrom(paramType);
        }

        private static int GetFirstExactMatchOptionalParams(MethodInfo m, Type[] types)
        {
            return IndexFirstTypeMatchOptionalParams(m, types, equals);
        }

        private static int GetFirstExactMatchVarargMethods(MethodInfo m, Type[] types)
        {
            return IndexFirstTestTypeMatchesVarArgs(m, types, equals, equals);
        }

        private static int GetLastExactMatchVarargMethods(MethodInfo m, Type[] types)
        {
            return IndexBeforeTransitionToNoMatchVarArgs(m, types, equals);
        }

        private static bool TestTypeMatchesOptionalParams(MethodInfo method, Type[] types, Func<Type, Type, bool> matchTest)
        {
            var parameters = method.GetParameters();
            if (parameters.Length == 0 && types.Length == 0) return true;
            if (types.Length > parameters.Length) return false; // this may be an issue with mix of default values and params keyword. So be it; feature later.
            if (types.Length < parameters.Length)
                if (!parameters[types.Length].IsOptional)
                    return false; // there remains at least one non-optional parameters that is missing.
            for (int i = 0; i < types.Length; i++)
            {
                if (!matchTest(parameters[i].ParameterType, types[i]))
                    return false;
            }
            return true;
        }

        private static int IndexFirstTypeMatchOptionalParams(MethodInfo method, Type[] types, Func<Type, Type, bool> matchTest)
        {
            var parameters = method.GetParameters();
            if (parameters.Length == 0 && types.Length == 0) return -1;
            if (types.Length > parameters.Length) return -1; // this may be an issue with mix of default values and params keyword. So be it; feature later.
            if (types.Length < parameters.Length)
                if (!parameters[types.Length].IsOptional)
                    return -1; // there remains at least one non-optional parameters that is missing.
            for (int i = 0; i < types.Length; i++)
            {
                if (matchTest(parameters[i].ParameterType, types[i]))
                    return i;
            }
            return -1;
        }

        private static bool TestTypeMatchesVarArgs(MethodInfo method, Type[] types, Func<Type, Type, bool> stdParamsMatchTest, Func<Type, Type, bool> paramsParamsMatchTest)
        {
            var parameters = method.GetParameters();
            if (parameters.Length == 0 && types.Length == 0) return true;
            if (types.Length < (parameters.Length - 1)) return false; // this may be an issue with mix of default values and params keyword. So be it; feature later.
            for (int i = 0; i < parameters.Length - 1; i++)
            {
                if (!stdParamsMatchTest(parameters[i].ParameterType, types[i]))
                    return false;
            }
            var arrayType = parameters[parameters.Length - 1].ParameterType;
            Type t;
            if (!arrayType.IsArray)
                throw new ArgumentException("Inconsistent - arguments should not be packed with a non-array method parameter");
            t = arrayType.GetElementType();
            for (int i = parameters.Length - 1; i < types.Length; i++)
            {
                if (!paramsParamsMatchTest(t, types[i]))
                    return false;
            }
            return true;
        }

        private static int IndexFirstTestTypeMatchesVarArgs(MethodInfo method, Type[] types, Func<Type, Type, bool> stdParamsMatchTest, Func<Type, Type, bool> paramsParamsMatchTest)
        {
            var parameters = method.GetParameters();
            if (parameters.Length == 0 && types.Length == 0) return -1;
            if (types.Length < (parameters.Length - 1)) return -1; // this may be an issue with mix of default values and params keyword. So be it; feature later.
            for (int i = 0; i < parameters.Length - 1; i++)
            {
                if (stdParamsMatchTest(parameters[i].ParameterType, types[i]))
                    return i;
            }
            var arrayType = parameters[parameters.Length - 1].ParameterType;
            Type t;
            if (!arrayType.IsArray)
                throw new ArgumentException("Inconsistent - arguments should not be packed with a non-array method parameter");
            t = arrayType.GetElementType();
            for (int i = parameters.Length - 1; i < types.Length; i++)
            {
                if (paramsParamsMatchTest(t, types[i]))
                    return i;
            }
            return -1;
        }

        /// <summary>
        /// Finds the last index of a parameter that matches a criteria, before a transition to false.
        /// </summary>
        /// <param name="method"></param>
        /// <param name="types"></param>
        /// <param name="matchTest"></param>
        /// <returns></returns>
        /// <remarks>
        /// Needed to cater for https://rclr.codeplex.com/workitem/30
        /// </remarks>
        private static int IndexBeforeTransitionToNoMatchVarArgs(MethodInfo method, Type[] types, Func<Type, Type, bool> matchTest)
        {
            var parameters = method.GetParameters();
            if (parameters.Length == 0 && types.Length == 0) return -1;
            if (types.Length < (parameters.Length - 1)) return -1; // this may be an issue with mix of default values and params keyword. So be it; feature later.
            bool hasHitMatch = false;
            for (int i = 0; i < parameters.Length - 1; i++)
            {
                if (!matchTest(parameters[i].ParameterType, types[i]))
                {
                    if (hasHitMatch)
                        return i - 1;
                }
                else
                    hasHitMatch = true;
            }
            var arrayType = parameters[parameters.Length - 1].ParameterType;
            Type t;
            if (!arrayType.IsArray)
                throw new ArgumentException("Inconsistent - arguments should not be packed with a non-array method parameter");
            t = arrayType.GetElementType();
            for (int i = parameters.Length - 1; i < types.Length; i++)
            {
                if (!matchTest(t, types[i]))
                {
                    if (hasHitMatch)
                        return i - 1;
                }
                else
                    hasHitMatch = true;
            }
            if (hasHitMatch) 
                // for cases where blah(object, int, params int[]) called with int, int, int, int, int
                return types.Length;
            else
                return -1;
        }

        public static bool HasOptionalParams(MethodInfo method)
        {
            var parameters = method.GetParameters();
            if (parameters.Length == 0) return false;
            var p = parameters[parameters.Length - 1];
            return p.IsOptional;
        }

        public static bool HasVarArgs(MethodInfo method)
        {
            var parameters = method.GetParameters();
            if (parameters.Length == 0) return false;
            var p = parameters[parameters.Length - 1];
            return IsVarArg(p);
        }

        public static bool IsVarArg(ParameterInfo p)
        {
            var pAttrib = p.GetCustomAttributes(typeof(ParamArrayAttribute), false);
            return pAttrib.Length > 0;
        }

        /// <summary>
        /// Gets all the methods of an object with a name that contains a specific string.
        /// </summary>
        /// <param name="obj">The object to reflect on, or its type</param>
        /// <param name="pattern">The case sensitive string to look for in member names</param>
        public static string[] GetMethods(object obj, string pattern)
        {
            Type type = obj as Type;
            if (type == null)
                type = obj.GetType();
            return getMethods(type, pattern, BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static);
        }

        /// <summary>
        /// Gets all the non-static public methods of an object with a name that contains a specific string.
        /// </summary>
        /// <param name="obj">The object to reflect on, or its type</param>
        /// <param name="pattern">The case sensitive string to look for in member names</param>
        public static string[] GetInstanceMethods(object obj, string pattern)
        {
            Type type = obj.GetType();
            return getMethods(type, pattern, BindingFlags.Public | BindingFlags.Instance);
        }

        /// <summary>
        /// Gets all the non-static public constructors of a class.
        /// </summary>
        /// <param name="typeName">type name</param>
        public static string[] GetConstructors(string typeName)
        {
            return GetConstructors(ClrFacade.GetType(typeName));
        }

        /// <summary>
        /// Gets all the non-static public constructors of a class.
        /// </summary>
        /// <param name="type">type</param>
        public static string[] GetConstructors(Type type)
        {
            return getConstructors(type, BindingFlags.Public | BindingFlags.Instance);
        }

        /// <summary>
        /// Gets all the static public methods of an object with a name that contains a specific string.
        /// </summary>
        /// <param name="obj">The object to reflect on, or its type</param>
        /// <param name="pattern">The case sensitive string to look for in member names</param>
        public static string[] GetStaticMethods(object obj, string pattern)
        {
            Type type = obj.GetType();
            return getMethods(type, pattern, BindingFlags.Public | BindingFlags.Static);
        }

        /// <param name="pattern">The case sensitive string to look for in member names</param>
        public static string[] GetStaticMethods(string typeName, string pattern)
        {
            Type type = ClrFacade.GetType(typeName);
            return getMethods(type, pattern, BindingFlags.Public | BindingFlags.Static);
        }

        /// <summary>
        /// Gets all the public fields of an object with a name that contains a specific string.
        /// </summary>
        /// <param name="obj">The object to reflect on, or its type</param>
        /// <param name="pattern">The case sensitive string to look for in member names</param>
        public static string[] GetFields(object obj, string pattern)
        {
            Type type = obj.GetType();
            return getFields(type, pattern, BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static);
        }

        /// <summary>
        /// Gets all the non-static public fields of an object with a name that contains a specific string.
        /// </summary>
        /// <param name="obj">The object to reflect on, or its type</param>
        /// <param name="pattern">The case sensitive string to look for in member names</param>
        public static string[] GetInstanceFields(object obj, string pattern)
        {
            Type type = obj.GetType();
            return getFields(type, pattern, BindingFlags.Public | BindingFlags.Instance);
        }

        /// <summary>
        /// Gets all the static fields of an object with a name that contains a specific string.
        /// </summary>
        /// <param name="obj">The object to reflect on, or its type</param>
        /// <param name="pattern">The case sensitive string to look for in member names</param>
        public static string[] GetStaticFields(object obj, string pattern)
        {
            Type type = obj.GetType();
            return getFields(type, pattern, BindingFlags.Public | BindingFlags.Static);
        }

        public static string[] GetStaticFields(string typeName, string pattern)
        {
            Type type = ClrFacade.GetType(typeName);
            return getFields(type, pattern, BindingFlags.Public | BindingFlags.Static);
        }

        public static string[] GetStaticFields(Type type, string pattern)
        {
            return getFields(type, pattern, BindingFlags.Public | BindingFlags.Static);
        }

        /// <summary>
        /// Gets the value of a field of an object.
        /// </summary>
        public static object GetFieldValue(object obj, string fieldname)
        {
            if (obj == null) throw new ArgumentNullException("obj must not be null", "obj");
            // FIXME: accessing private fields should be discouraged.
            var field = obj.GetType().GetField(fieldname, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
            if (field == null) throw new ArgumentException(
                string.Format("Field {0} not found on object of type {1}", fieldname, obj.GetType().FullName),
                "fieldname");
            return field.GetValue(obj);
        }

        /// <summary>
        /// Gets all the properties of an object with a name that contains a specific string.
        /// </summary>
        /// <param name="obj">The object to reflect on, or its type</param>
        /// <param name="pattern">The case sensitive string to look for in member names</param>
        public static string[] GetProperties(object obj, string pattern)
        {
            Type type = obj.GetType();
            return getProperties(type, pattern, BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static);
        }

        /// <summary>
        /// Gets all the non-static public properties of an object with a name that contains a specific string.
        /// </summary>
        /// <param name="obj">The object to reflect on, or its type</param>
        /// <param name="pattern">The case sensitive string to look for in member names</param>
        public static string[] GetInstanceProperties(object obj, string pattern)
        {
            Type type = obj.GetType();
            return getProperties(type, pattern, BindingFlags.Public | BindingFlags.Instance);
        }

        /// <summary>
        /// Gets all the static public properties of an object with a name that contains a specific string.
        /// </summary>
        /// <param name="obj">The object to reflect on, or its type</param>
        /// <param name="pattern">The case sensitive string to look for in member names</param>
        public static string[] GetStaticProperties(object obj, string pattern)
        {
            Type type = obj.GetType();
            return getProperties(type, pattern, BindingFlags.Public | BindingFlags.Static);
        }

        public static string[] GetStaticProperties(string typeName, string pattern)
        {
            Type type = ClrFacade.GetType(typeName);
            return getProperties(type, pattern, BindingFlags.Public | BindingFlags.Static);
        }

        /// <summary>
        /// Gets the value of a property of an object.
        /// </summary>
        public static object GetPropertyValue(object obj, string propname)
        {
            if (obj == null) throw new ArgumentNullException("obj must not be null", "obj");
            // FIXME: accessing private fields should be discouraged.
            var field = obj.GetType().GetProperty(propname, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
            if (field == null) throw new ArgumentException(
                string.Format("Property {0} not found on object of type {1}", propname, obj.GetType().FullName),
                "fieldname");
            return field.GetValue(obj, null);
        }

		public static string[] GetEnumNames(string enumTypename)
		{
			var t = ClrFacade.GetType(enumTypename);
            if (t==null) throw new ArgumentException(String.Format("Type not found: {0}", enumTypename));
			return GetEnumNames(t);
		}

		public static string[] GetEnumNames(Type enumType)
		{
			if(enumType==null) throw new ArgumentNullException();
			if(typeof(Enum).IsAssignableFrom(enumType) == false)
				throw new ArgumentException(string.Format("{0} is not the type of an Enum", enumType.ToString()));
			return System.Enum.GetNames(enumType);
		}

		public static string[] GetEnumNames(Enum e)
		{
			return GetEnumNames(e.GetType());
		}

        public static string[] GetInterfacesFullnames(Type type)
        {
            var ifaces = type.GetInterfaces();
            return Array.ConvertAll(ifaces, x => x.FullName);
        }

        public static string[] GetDeclaredMethodNames(Type type, BindingFlags bindings = BindingFlags.DeclaredOnly | BindingFlags.Public  | BindingFlags.Instance )
        {
            var methods = type.GetMethods(bindings);
            return Array.ConvertAll(methods, x => x.Name);
        }

        private static string[] getFields(Type type, string pattern, BindingFlags flags)
        {
            var fieldNames =
                from field in type.GetFields(flags)
                where field.Name.Contains(pattern)
                select field.Name;
            return sort(fieldNames.ToArray());
        }

        private static string[] sort(string[] result)
        {
            Array.Sort(result);
            return result;
        }

        private static string[] getProperties(Type type, string pattern, BindingFlags flags)
        {
            var propNames =
                from property in type.GetProperties(flags)
                where property.Name.Contains(pattern)
                select property.Name;
            return sort(propNames.ToArray());
        }

        private static string[] getMethods(Type type, string pattern, BindingFlags flags)
        {
            var methNames =
                from method in type.GetMethods(flags)
                where method.Name.Contains(pattern)
                select method.Name;
            return sort(methNames.ToArray());
        }

        private static string[] getConstructors(Type type, BindingFlags flags)
        {
            return sort(Array.ConvertAll(type.GetConstructors(flags), x => summarizeConstructor(x)));
        }

        private static string[] summarize(MemberInfo[] members)
        {
            var result = new string[members.Length];
            for (int i = 0; i < members.Length; i++)
                result[i] = summarize(members[i]);
            return result;
        }

        private static string summarize(MemberInfo member)
        {
            var ctor = member as ConstructorInfo;
            if (ctor != null) return summarizeConstructor(ctor);
            var field = member as FieldInfo;
            if (field != null) return summarizeField(field);
            var prop = member as PropertyInfo;
            if (prop != null) return summarizeProperty(prop);
            var method = member as MethodInfo;
            if (method != null) return summarizeMethod(method);
            throw new ArgumentException("MemberInfo is not a constructor, field, property of method-info, but of type {1}", member.GetType().ToString());
        }

        private static string summarizeConstructor(ConstructorInfo ctor)
        {
            var result = new StringBuilder();
            addMethodBaseInfo(ctor, result);
            result.Append(string.Format("Constructor: {0}", ctor.Name));

            var parameters = ctor.GetParameters();
            addParametersSummary(result, parameters);
            return result.ToString();
        }

        private static void addMethodBaseInfo(MethodBase methodBase, StringBuilder result)
        {
            if (methodBase.IsGenericMethod) result.Append("Generic, ");
            if (methodBase.IsGenericMethodDefinition) result.Append("Generic definition, ");
            if (methodBase.IsStatic) result.Append("Static, ");
            if (methodBase.IsAbstract) result.Append("abstract, ");
        }

        private static string summarizeMethod(MethodInfo method)
        {
            var result = new StringBuilder();
            addMethodBaseInfo(method, result);
            result.Append(string.Format("Method: {0} {1}",
                                       method.ReturnType.Name, method.Name));

            var parameters = method.GetParameters();
            addParametersSummary(result, parameters);
            return result.ToString();
        }

        private static void addParametersSummary(StringBuilder result, ParameterInfo[] parameters)
        {
            if (parameters.Length > 0)
                result.Append(", ");
            result.Append(SummarizeTypes(parameters));
        }

        internal static string SummarizeTypes(ParameterInfo[] parameters)
        {
            var types = parameters.Select(p => p.ParameterType).ToArray();
            return SummarizeTypes(types);
        }

        internal static string SummarizeTypes(Type[] types)
        {
            var result = new StringBuilder();
            for (int i = 0; i < (types.Length - 1); i++)
            {
                result.Append(types[i].Name);
                result.Append(", ");
            }
            if (types.Length > 0)
                result.Append(types[types.Length - 1].Name);
            return result.ToString();
        }

        internal static string[] GetMethodParameterTypes(MethodBase method)
        {
            var result = new List<string>();
            var parameters = method.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                result.Add(parameters[i].ParameterType.FullName);
            }
            return result.ToArray();
        }

        private static string summarizeProperty(PropertyInfo prop)
        {
            return string.Format("Property {0}, {1}, can write: {2}", prop.Name, prop.PropertyType.Name, prop.CanWrite);
        }

        private static string summarizeField(FieldInfo field)
        {
            return string.Format("Field {0}, {1}", field.Name, field.FieldType.Name);
        }

        internal static void ThrowMissingMethod(Type classType, string methodName, string modifier, Type[] types)
        {
            var s = types.Length == 0 ? 
                "without method parameters" : "for method parameters " + SummarizeTypes(types);
            throw new MissingMethodException(String.Format("Could not find a suitable {2} method {0} on type {1} {3}", 
                methodName, classType.FullName, modifier, s));
        }
    }
}
