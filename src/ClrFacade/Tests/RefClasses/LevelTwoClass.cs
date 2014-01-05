using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Rclr.Tests.RefClasses
{
    public class LevelTwoClass : LevelOneClass, InterfaceTwo
    {
        public virtual string IfBaseOneString { get; set; }

        public virtual string IfBaseTwoString { get; set; }
        
        public string IfBaseTwoMethod()
        {
            return "LevelTwoClass::IfBaseTwoMethod()";
        }

        public string IfBaseTwoMethod(string par)
        {
            return "LevelTwoClass::IfBaseTwoMethod(string)";
        }
    }
}
