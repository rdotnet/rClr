#define RDN15
using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Text;
using RDotNet;
using Environment = System.Environment;

namespace DummyApp
{
	class MainClass
	{
        static void Main(string[] args)
        {

            var rHome = "/usr/lib/R";
            string path = Environment.GetEnvironmentVariable("PATH") ?? String.Empty;
			// Note that using /usr/lib where a libR.so symlink exists is not good enough
			path = string.Concat(path, ":","/usr/lib/R/lib");
			Environment.SetEnvironmentVariable("R_HOME", rHome);
			Environment.SetEnvironmentVariable("PATH", path);

            Console.WriteLine("R init: creating R engine");
            REngine rEngine = REngine.CreateInstance("RDotNet");
            rEngine.Initialize();
            Console.WriteLine("Created rEngine: " + rEngine.ToString());

            // simple arithmetic test
            const string arithmeticExpression = "2 + 14 * 7";
            var result = rEngine.Evaluate(arithmeticExpression).AsNumeric().ToArray();
            Console.WriteLine(arithmeticExpression + " = " + result[0]);

            // test the problematic CreateNumericVector method 
            // Missing method RDotNet.REngineExtension::CreateNumericVector(REngine,IEnumerable`1<double>) in assembly /data/col52j/calibration-files/bin/Release/R.NET.dll
            // values <- 0:99
            double[] values = new double[100];
            for (int i = 0; i < values.Length; i++)
                values[i] = i;
            rEngine.SetSymbol("values", rEngine.CreateNumericVector(values));
            // calculate the sum
            // sum(values) # 4950
            string sum = "sum(values)";
#if RDN15
            result = rEngine.Evaluate(sum).AsNumeric().ToArray();
#else
            result = rEngine.EagerEvaluate(sum).AsNumeric().ToArray();
#endif
            Console.WriteLine("Sum of integer range 0:99 = " + result[0]);
        }
	}
}
