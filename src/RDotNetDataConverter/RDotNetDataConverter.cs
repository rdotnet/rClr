using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using RDotNet;
using RDotNet.NativeLibrary;

namespace Rclr
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void Rf_error(string msg);

    public class RDotNetDataConverter : IDataConverter
    {
        private RDotNetDataConverter ()
        {
            SetupREngine ();
            // The Mono API already has some unhandled exception reporting. 
            // TODO Use the following if it works well for both CLRuntimes.
#if !MONO
            SetupExceptionHandling();
#endif
            ConvertVectors = true;
            ConvertValueTypes = true;
        }

        private void SetupExceptionHandling()
        {
            AppDomain.CurrentDomain.UnhandledException += OnUnhandleException;
        }

        private void OnUnhandleException(object sender, UnhandledExceptionEventArgs e)
        {
            Exception ex = (Exception)e.ExceptionObject;
            Error(ClrFacade.FormatException(ex));
        }

        public void Error(string msg)
        {
            throw new NotSupportedException();
            //engine.Error(msg);
        }

        /// <summary>
        /// Enable/disable the use of this data converter in the R-CLR interop data marshalling.
        /// </summary>
        public static void SetRDotNet(bool setit)
        {
            if (setit)
                ClrFacade.DataConverter = GetInstance();
            else
                ClrFacade.DataConverter = null;
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
            if (obj == null) 
                return null;
            var sexp = obj as SymbolicExpression;
            if (sexp != null)
                return sexp.DangerousGetHandle();

            sexp = TryConvertToSexp(obj);

            if (sexp == null)
                return obj;
            return sexp.DangerousGetHandle();
        }

        public object ConvertFromR(IntPtr pointer, int sexptype)
        {
            return new DataFrame(engine, pointer);
        }

        public bool ConvertVectors { get; set; }
        public bool ConvertValueTypes { get; set; }

        private void SetupREngine()
        {
			string dll = null;
			switch (NativeUtility.GetPlatform())
			{
				case PlatformID.Win32NT:
				case PlatformID.MacOSX:
					break;
				case PlatformID.Unix:
					// trying to fix http://r2clr.codeplex.com/workitem/49
					// but this is likely not to be enough (architecture dependent likely)
					dll = "/usr/lib/libR.so"; 
					break;
				default:
					throw new NotSupportedException();
			}

            engine = REngine.CreateInstance("RDotNetDataConverter", dll: dll);
            engine.Initialize(setupMainLoop: false);
        }

        private static RDotNetDataConverter singleton;

        private static RDotNetDataConverter GetInstance()
        {
            // Make sure this is set only once (RDotNet known limitation to one engine per session, effectively a singleton).
            if (singleton == null)
                singleton = new RDotNetDataConverter();
            return singleton;
        }

        private SymbolicExpression TryConvertToSexp(object obj)
        {
            SymbolicExpression sHandle = null;
            if (ConvertValueTypes)
                sHandle = testValueTypes(obj);
            if (sHandle == null && ConvertVectors)
                sHandle = testVector(obj);
            if (sHandle == null)
                sHandle = testMatrix(obj);
            if (sHandle == null)
                sHandle = testDictionary(obj);
            return sHandle;
        }

        private SymbolicExpression ConvertToSexp(object obj)
        {
            if (obj == null) return null;
            var result = TryConvertToSexp(obj);
            if(result==null)
                throw new NotSupportedException(string.Format("Cannot yet expose type {0} as a SEXP", obj.GetType().FullName));
            return result;
        }

        private SymbolicExpression testDictionary(object obj)
        {
            var dss = obj as IDictionary<string, string>;
            if (dss != null)
            {
                var values = new CharacterVector(this.engine, dss.Values);
                SetAttribute(values, dss.Keys.ToArray());
                return values.AsList();
            }
            var dsa = obj as IDictionary<string, double[]>;
            if (dsa != null)
            {
                var values = ConvertAll(dsa.Values.ToArray());
                SetAttribute(values, dsa.Keys.ToArray());
                return values.AsList();
            }
            var dso = obj as IDictionary<string, object>;
            if (dso != null)
            {
                var values = ConvertAll(dso.Values.ToArray());
                SetAttribute(values, dso.Keys.ToArray());
                return values.AsList();
            }
            return null;
        }

        private SymbolicExpression ConvertAll(object[] objects)
        {
            var sexpArray = new SymbolicExpression[objects.Length];
            for (int i = 0; i < objects.Length; i++)
                sexpArray[i] = ConvertToSexp(objects[i]);
            return new GenericVector(engine, sexpArray);
        }

        private SymbolicExpression testValueTypes(object obj)
        {
            SymbolicExpression sHandle = null;
            if (obj != null)
            {
                if (obj is Array)
                {
                    if (obj is DateTime[])
                        sHandle = toNumericArray((DateTime[])obj);
                    else if (obj is TimeSpan[])
                        sHandle = toNumericArray((TimeSpan[])obj);
                }
                else
                {
                    if (obj is DateTime)
                        sHandle = toNumericArray(new[] { (DateTime)obj });
                    else if (obj is TimeSpan)
                        sHandle = toNumericArray(new[] { (TimeSpan)obj });
                }
            }
            return sHandle;
        }

        private SymbolicExpression testVector(object obj)
        {
            SymbolicExpression sHandle = null;
            if (obj != null)
            {
                if (obj is float[])
                    sHandle = toNumericArray((float[])obj);
                else if (obj is double[])
                    sHandle = toNumericArray((double[])obj);
                else if (obj is int[])
                    sHandle = toNumericArray((int[])obj);
            }
            return sHandle;
        }

        private SymbolicExpression testMatrix(object obj)
        {
            SymbolicExpression sHandle = null;
            if (obj != null)
            {
                if (obj is float[][])
                    sHandle = toMatrix((float[][])obj);
                else if (obj is double[][])
                    sHandle = toMatrix((double[][])obj);
                else if (obj is string[][])
                    sHandle = toMatrix((string[][])obj);
                else if (obj is float[,])
                    sHandle = toMatrix((float[,])obj);
                else if (obj is double[,])
                    sHandle = toMatrix((double[,])obj);
                else if (obj.GetType() == typeof(string[,]))
                    sHandle = toMatrix((string[,])obj);
            }
            return sHandle;
        }

        private SymbolicExpression toNumericArray(double[] array)
        {
            return engine.CreateNumericVector(array);
        }

        private SymbolicExpression toNumericArray(float[] array)
        {
            return toNumericArray(Array.ConvertAll(array, x => (double)x));
        }

        private SymbolicExpression toNumericArray(int[] array)
        {
            return engine.CreateIntegerVector(array);
        }

        private SymbolicExpression toNumericArray(DateTime[] array)
        {
            var doubleArray = Array.ConvertAll(array, ClrFacade.GetRPosixCtDoubleRepresentation);
            var result = toNumericArray(doubleArray);
            SetClassAttribute(result, "POSIXct", "POSIXt");
            SetTzoneAttribute(result, "UTC");
            return result;
        }

        private SymbolicExpression toNumericArray(TimeSpan[] array)
        {
            var doubleArray = Array.ConvertAll(array, ( x => x.TotalSeconds));
            var result = toNumericArray(doubleArray);
            SetClassAttribute(result, "difftime"); // class(as.difftime(3.5, units='secs'))
            SetUnitsAttribute(result, "secs");  // unclass(as.difftime(3.5, units='secs'))
            return result;
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
            SetAttribute(sexp, classes, attributeName:"class");
        }

        private void SetAttribute(SymbolicExpression sexp, string[] attribValues, string attributeName = "names")
        {
            var names = new CharacterVector(engine, attribValues);
            sexp.SetAttribute(attributeName, names);
        }

        private NumericMatrix toMatrix(float[][] array)
        {
            return createNumericMatrix(array.ToDoubleRect());
        }

        private NumericMatrix toMatrix(double[][] array)
        {
            return createNumericMatrix(array.ToDoubleRect());
        }

        private CharacterMatrix toMatrix(string[][] array)
        {
            return createCharacterMatrix(array.ToStringRect());
        }

        private NumericMatrix toMatrix(float[,] array)
        {
            return createNumericMatrix(array.ToDoubleRect());
        }

        private NumericMatrix toMatrix(double[,] array)
        {
            return createNumericMatrix(array);
        }

        private CharacterMatrix toMatrix(string[,] array)
        {
            return createCharacterMatrix(array);
        }

        private CharacterMatrix createCharacterMatrix(string[,] array)
        {
            return engine.CreateCharacterMatrix(array);
        }

        private NumericMatrix createNumericMatrix(double[,] array)
        {
            return engine.CreateNumericMatrix(array);
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
            return NativeUtility.GetRDllFileName();
        }

        public REngine engine;

        public REngine GetEngine()
        {
            return engine;
        }

    }
}