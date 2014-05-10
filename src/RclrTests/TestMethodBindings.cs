using System;
using System.Reflection;
using Rclr;
using System.Collections.Generic;
using Xunit;

namespace RclrTests
{
    /// <summary>
    /// Do not modify the .cs file: T4 generated class to support the unit tests for method binding
    /// </summary>
	public class TestMethodBindings
    {


		[Fact]
		public void TestMethodBindingOptionalParameters()
		{
			var tname = typeof(TestMethodBinding).FullName;
			int anInt = 1;
			object anObject = new Object();

            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anObject, anObject, anObject, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anObject, anObject, anObject, anObject, anObject })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anObject, anObject, anObject, anObject, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anObject, anObject, anObject, anObject, anInt })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anObject, anObject, anObject, anInt, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anObject, anObject, anObject, anInt, anObject })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anObject, anObject, anObject, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anObject, anObject, anObject, anInt, anInt })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anObject, anObject, anInt, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anObject, anObject, anInt, anObject, anObject })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anObject, anObject, anInt, anObject, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anObject, anObject, anInt, anObject, anInt })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anObject, anObject, anInt, anInt, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anObject, anObject, anInt, anInt, anObject })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anObject, anObject, anInt, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anObject, anObject, anInt, anInt, anInt })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anObject, anInt, anObject, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anObject, anInt, anObject, anObject, anObject })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anObject, anInt, anObject, anObject, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anObject, anInt, anObject, anObject, anInt })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anObject, anInt, anObject, anInt, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anObject, anInt, anObject, anInt, anObject })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anObject, anInt, anObject, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anObject, anInt, anObject, anInt, anInt })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anObject, anInt, anInt, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anObject, anInt, anInt, anObject, anObject })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anObject, anInt, anInt, anObject, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anObject, anInt, anInt, anObject, anInt })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anObject, anInt, anInt, anInt, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anObject, anInt, anInt, anInt, anObject })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anObject, anInt, anInt, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anObject, anInt, anInt, anInt, anInt })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anInt, anObject, anObject, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anInt, anObject, anObject, anObject, anObject })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anInt, anObject, anObject, anObject, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anInt, anObject, anObject, anObject, anInt })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anInt, anObject, anObject, anInt, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anInt, anObject, anObject, anInt, anObject })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anInt, anObject, anObject, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anInt, anObject, anObject, anInt, anInt })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anInt, anObject, anInt, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anInt, anObject, anInt, anObject, anObject })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anInt, anObject, anInt, anObject, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anInt, anObject, anInt, anObject, anInt })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anInt, anObject, anInt, anInt, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anInt, anObject, anInt, anInt, anObject })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anInt, anObject, anInt, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anInt, anObject, anInt, anInt, anInt })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anInt, anInt, anObject, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anInt, anInt, anObject, anObject, anObject })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anInt, anInt, anObject, anObject, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anInt, anInt, anObject, anObject, anInt })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anInt, anInt, anObject, anInt, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anInt, anInt, anObject, anInt, anObject })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anInt, anInt, anObject, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anInt, anInt, anObject, anInt, anInt })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anInt, anInt, anInt, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anInt, anInt, anInt, anObject, anObject })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anInt, anInt, anInt, anObject, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anInt, anInt, anInt, anObject, anInt })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anInt, anInt, anInt, anInt, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anInt, anInt, anInt, anInt, anObject })
            );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithOptionalArguments(anInt, anInt, anInt, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithOptionalArguments", new object[] { anInt, anInt, anInt, anInt, anInt })
            );
		}
	}
}

