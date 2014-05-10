using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Rclr.Tests
{
    /// <summary>
    /// Utilities used for testing purposes.
    /// </summary>
    public static class TestUtilities
    {
        /// <summary>
        /// A string concatenator
        /// </summary>
        /// <param name="strs"></param>
        /// <param name="sep"></param>
        /// <returns></returns>
        public static string Concat(IEnumerable<string> values, string sep)
        {
            var strs = values.ToArray();
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < strs.Length - 1; i++)
            {
                sb.Append(strs[i]); sb.Append(sep);
            }
            sb.Append(strs[strs.Length - 1]);
            return sb.ToString();
        }

        public static string[] BuildCombinatorialTestCases(string valForTrue, string valForFalse, int np = 5, string sep=", ")
        {
            int cases = (int)Math.Pow(2, np);
            var result = new string[cases];
            for (int i = 0; i < cases; i++)
            {
                bool[] s = Convert.ToString(i, 2).ToCharArray().Select(x => x == '1').ToArray();
                bool[] b = new bool[np];
                Array.Copy(s, 0, b, b.Length - s.Length, s.Length);
                string[] paramsArray = Array.ConvertAll(b, x => (x ? valForTrue : valForFalse));
                string paramsBody = TestUtilities.Concat(paramsArray, sep);
                result[i] = paramsBody;
            }
            return result;
        }
    }
}
