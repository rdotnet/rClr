using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Rclr.Tests.RefClasses
{
    public class LevelOneClass : BaseAbstractClassOne, InterfaceOne
    {
        public override string AbstractMethod()
        {
            return "LevelOneClass::AbstractMethod()";
        }

        public override string AbstractMethod(string arg)
        {
            return "LevelOneClass::AbstractMethod(string)";
        }

        string InterfaceOne.IfOneString
        {
            get ; set ;
        }

        string InterfaceOne.IfOneStringGetter
        {
            get { return "Explicit LevelOneClass::InterfaceOne.IfOneStringGetter"; }
        }
    }
}
