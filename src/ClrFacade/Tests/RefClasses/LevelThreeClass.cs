using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Rclr.Tests.RefClasses
{
    public class LevelThreeClass : LevelTwoClass
    {
        public override string AbstractMethod()
        {
            return "LevelThreeClass::AbstractMethod()";
        }

        public override string VirtualMethod()
        {
            return "LevelThreeClass::VirtualMethod()";
        }

        public override string IfBaseOneString
        {
            get
            {
                return base.IfBaseOneString;
            }
            set
            {
                base.IfBaseOneString = "Overriden LevelThreeClass::IfBaseOneString " + value;
            }
        }
    }
}
