using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using RDotNet;

namespace Rclr
{
    public class RdotnetDataConverterTests
    {
        public class MemTestObjectRDotnet : RDotNet.NumericVector
        {
            public static int counter = 0;
            public MemTestObjectRDotnet(double[] values)
                : base(RdotnetDataConverterTests.REngine, values)
            {
                counter++;
            }
            ~MemTestObjectRDotnet() { counter--; }
        }

        public static int GetMemTestObjCounterRDotnet()
        {
            return MemTestObjectRDotnet.counter;
        }

        public static object CreateMemTestObjRDotnet()
        {
            return new MemTestObjectRDotnet(Rclr.TestCases.CreateNumArray());
        }

        internal static REngine REngine
        {
            get
            {
                var rdotnetconverter = ClrFacade.DataConverter as RDotNetDataConverter;
                if (rdotnetconverter == null)
                    return null;
                else
                    return rdotnetconverter.engine;
            }
        }
    }
}
