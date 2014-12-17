using System;
using Rclr;
using RDotNet;

namespace TestApp
{
    class MainClass
    {
        public static void Main(string[] args)
        {
            // An application with an entry point.
            // This is (seems) required by MonoDevelop to run the debugger in listen mode from a solution 
            var tzIdR_AUest = "Australia/Sydney";

            var timeZoneId = tzIdR_AUest;
            var tz = TimeZoneInfo.FindSystemTimeZoneById(timeZoneId);

            var e = REngine.GetInstance();
            var pi = e.CreateNumeric(3.1415);
            var sxpw = new SymbolicExpressionWrapper(pi);
            var obj = sxpw.ToClrEquivalent();

            var yep = (obj is double);
            Console.WriteLine((double)obj);

        }
    }
}
