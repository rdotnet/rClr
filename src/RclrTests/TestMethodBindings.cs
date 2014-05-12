using System;
using System.Reflection;
using Rclr;
using System.Collections.Generic;
using Xunit;
using Rclr.Tests.RefClasses;

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
			double aDouble = Math.PI;
			object anObject = new Object();

            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anObject, anObject, anObject, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anObject, anObject, anObject, anObject, anObject })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anObject, anObject, anObject, anObject, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anObject, anObject, anObject, anObject, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anObject, anObject, anObject, anInt, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anObject, anObject, anObject, anInt, anObject })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anObject, anObject, anObject, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anObject, anObject, anObject, anInt, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anObject, anObject, anInt, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anObject, anObject, anInt, anObject, anObject })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anObject, anObject, anInt, anObject, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anObject, anObject, anInt, anObject, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anObject, anObject, anInt, anInt, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anObject, anObject, anInt, anInt, anObject })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anObject, anObject, anInt, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anObject, anObject, anInt, anInt, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anObject, anInt, anObject, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anObject, anInt, anObject, anObject, anObject })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anObject, anInt, anObject, anObject, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anObject, anInt, anObject, anObject, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anObject, anInt, anObject, anInt, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anObject, anInt, anObject, anInt, anObject })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anObject, anInt, anObject, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anObject, anInt, anObject, anInt, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anObject, anInt, anInt, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anObject, anInt, anInt, anObject, anObject })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anObject, anInt, anInt, anObject, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anObject, anInt, anInt, anObject, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anObject, anInt, anInt, anInt, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anObject, anInt, anInt, anInt, anObject })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anObject, anInt, anInt, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anObject, anInt, anInt, anInt, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anObject, anObject, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anObject, anObject, anObject, anObject })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anObject, anObject, anObject, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anObject, anObject, anObject, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anObject, anObject, anInt, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anObject, anObject, anInt, anObject })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anObject, anObject, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anObject, anObject, anInt, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anObject, anInt, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anObject, anInt, anObject, anObject })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anObject, anInt, anObject, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anObject, anInt, anObject, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anObject, anInt, anInt, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anObject, anInt, anInt, anObject })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anObject, anInt, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anObject, anInt, anInt, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anObject, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anInt, anObject, anObject, anObject })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anObject, anObject, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anInt, anObject, anObject, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anObject, anInt, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anInt, anObject, anInt, anObject })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anObject, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anInt, anObject, anInt, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anInt, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anInt, anInt, anObject, anObject })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anInt, anObject, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anInt, anInt, anObject, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anInt, anInt, anObject),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anInt, anInt, anInt, anObject })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anInt, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anInt, anInt, anInt, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(aDouble, aDouble, aDouble, aDouble, aDouble),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { aDouble, aDouble, aDouble, aDouble, aDouble })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(aDouble, aDouble, aDouble, aDouble, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { aDouble, aDouble, aDouble, aDouble, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(aDouble, aDouble, aDouble, anInt, aDouble),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { aDouble, aDouble, aDouble, anInt, aDouble })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(aDouble, aDouble, aDouble, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { aDouble, aDouble, aDouble, anInt, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(aDouble, aDouble, anInt, aDouble, aDouble),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { aDouble, aDouble, anInt, aDouble, aDouble })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(aDouble, aDouble, anInt, aDouble, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { aDouble, aDouble, anInt, aDouble, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(aDouble, aDouble, anInt, anInt, aDouble),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { aDouble, aDouble, anInt, anInt, aDouble })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(aDouble, aDouble, anInt, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { aDouble, aDouble, anInt, anInt, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(aDouble, anInt, aDouble, aDouble, aDouble),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { aDouble, anInt, aDouble, aDouble, aDouble })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(aDouble, anInt, aDouble, aDouble, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { aDouble, anInt, aDouble, aDouble, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(aDouble, anInt, aDouble, anInt, aDouble),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { aDouble, anInt, aDouble, anInt, aDouble })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(aDouble, anInt, aDouble, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { aDouble, anInt, aDouble, anInt, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(aDouble, anInt, anInt, aDouble, aDouble),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { aDouble, anInt, anInt, aDouble, aDouble })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(aDouble, anInt, anInt, aDouble, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { aDouble, anInt, anInt, aDouble, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(aDouble, anInt, anInt, anInt, aDouble),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { aDouble, anInt, anInt, anInt, aDouble })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(aDouble, anInt, anInt, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { aDouble, anInt, anInt, anInt, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, aDouble, aDouble, aDouble, aDouble),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, aDouble, aDouble, aDouble, aDouble })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, aDouble, aDouble, aDouble, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, aDouble, aDouble, aDouble, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, aDouble, aDouble, anInt, aDouble),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, aDouble, aDouble, anInt, aDouble })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, aDouble, aDouble, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, aDouble, aDouble, anInt, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, aDouble, anInt, aDouble, aDouble),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, aDouble, anInt, aDouble, aDouble })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, aDouble, anInt, aDouble, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, aDouble, anInt, aDouble, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, aDouble, anInt, anInt, aDouble),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, aDouble, anInt, anInt, aDouble })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, aDouble, anInt, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, aDouble, anInt, anInt, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, aDouble, aDouble, aDouble),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anInt, aDouble, aDouble, aDouble })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, aDouble, aDouble, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anInt, aDouble, aDouble, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, aDouble, anInt, aDouble),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anInt, aDouble, anInt, aDouble })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, aDouble, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anInt, aDouble, anInt, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anInt, aDouble, aDouble),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anInt, anInt, aDouble, aDouble })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anInt, aDouble, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anInt, anInt, aDouble, anInt })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anInt, anInt, aDouble),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anInt, anInt, anInt, aDouble })   );
            Assert.Equal(
                TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anInt, anInt, anInt),
                ClrFacade.CallStaticMethod(tname, "SomeMethodWithVarArgs", new object[] { anInt, anInt, anInt, anInt, anInt })   );

            Assert.Equal(
                TestMethodBinding.MultipleMatchVarArgs(anObject, new LevelOneClass(), anObject, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "MultipleMatchVarArgs", new object[] { anObject, new LevelOneClass(), anObject, anObject, anObject })  );

            Assert.Equal(
                TestMethodBinding.MultipleMatchVarArgs(anObject, new LevelTwoClass(), anObject, anObject, anObject),
                ClrFacade.CallStaticMethod(tname, "MultipleMatchVarArgs", new object[] { anObject, new LevelTwoClass(), anObject, anObject, anObject })  );

            Assert.Throws<AmbiguousMatchException>(
			  () => { ClrFacade.CallStaticMethod(tname, "MultipleMatchVarArgs", new object[] { anObject, new LevelThreeClass(), anObject, anObject, anObject }); } );




		}
	}
}

