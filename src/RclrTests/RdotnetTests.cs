using System;
using Xunit;
using RDotNet;
using Rclr;

namespace RclrTests
{
    public class RdotnetTests
    {
        [Fact]
        public void TestSexpWrapper()
        {
            var e = REngine.GetInstance();
            var pi = e.CreateNumeric(3.1415);
            var sxpw = new SymbolicExpressionWrapper(pi);
        }
    }
}

