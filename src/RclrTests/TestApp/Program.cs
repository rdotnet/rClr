using System;

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

        }
    }
}
