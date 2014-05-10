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
        public static string Concat(string[] strs, string sep)
        {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < strs.Length - 1; i++)
            {
                sb.Append(strs[i]); sb.Append(sep);
            }
            sb.Append(strs[strs.Length - 1]);
            return sb.ToString();
        }
    }
}
