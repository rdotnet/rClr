using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Reflection;
using System.Text;

namespace Rclr
{
    /// <summary>
    /// A class that is part of the unit test for the core of rClr. See r2clr\packages\rClr\inst\tests\test.r
    /// </summary>
    public class TestCases
    {
        public static bool GetTrue()
        {
            return true;
        }

        public static bool GetFalse()
        {
            return false;
        }

        public static bool IsTrue(bool arg)
        {
            return arg;
        }

        public static bool DateEquals(DateTime dt, string isoDateTime)
        {
            var res = DateEquals(dt, DateTime.Parse(isoDateTime));
            if (!res)
            {
                Console.WriteLine(isoDateTime);
            }
            return res;
        }

        public static bool UtcDateEquals(DateTime dt, string isoDateTime, string timeZoneId = "UTC")
        {
            if (dt.Kind != DateTimeKind.Utc)
            {
                Console.WriteLine("Tested date time is not of Kind 'Utc'");
                return false;
            }
            var expected = UtcDateForTimeZone(isoDateTime, timeZoneId);
            var res = DateEquals(dt, expected);
            if (!res)
            {
                Console.WriteLine(isoDateTime);
            }
            return res;
        }

        public static bool UtcDateEquals(DateTime[] dts, string[] isoDateTimes, string timeZoneId = "UTC")
        {
            if (dts.Length != isoDateTimes.Length)
                return false;

            for (int i = 0; i < dts.Length; i++)
            {
                if(!UtcDateEquals(dts[i], isoDateTimes[i], timeZoneId))
                    return false;
            }
            return true;
        }

        public static bool DateEquals(DateTime dt1, DateTime dt2)
        {
            var res = (dt1 == dt2);
            if (!res)
            {
                Console.WriteLine("DateEqualsFails");
                Console.WriteLine(dt1.ToString());
            }
            return res;
        }

        public static bool DateEquals(DateTime[] dt1, DateTime[] dt2)
        {
            if (dt1.Length != dt2.Length)
                return false;
            for (int i = 0; i < dt1.Length; i++)
            {
                if(!dt1[i].Equals(dt2[i]))
                    return false;
            }
            return true;
        }

        public static void SinkDateTime(DateTime ignored)
        {
            // do nothing
        }

        /// <summary>
        /// A method to help test whether memory is not leaking in rClr when passing objects.
        /// </summary>
        public static void SinkLargeObject(object obj)
        {
            int breakpoint = 1;
        }

        public static int GetWorkingSetMemoryMegabtyes()
        {
            return (int)(Process.GetCurrentProcess().WorkingSet64 / 1e6);
        }

        public static int GetPrivateMemoryMegabtyes()
        {
            return (int)(Process.GetCurrentProcess().PrivateMemorySize64/ 1e6);
        }

        public static DateTime CreateDate(string isoDateTime, string datetimeKind = "")
        {
            var res = DateTime.Parse(isoDateTime);
            if (!string.IsNullOrEmpty(datetimeKind))
            {
                DateTimeKind dtk;
                if (Enum.TryParse(datetimeKind, out dtk))
                    res = new DateTime(res.Ticks, dtk);
            }
            return res;
        }

        public static DateTime[] CreateDateArray(string isoDateTimeStart, int numDays)
        {
            var startDate = CreateDate(isoDateTimeStart);
            DateTime[] result = CreateDailySequence(numDays, startDate);
            return result;
        }

        public static DateTime UtcDateForTimeZone(string isoDateTime, string timeZoneId = "UTC")
        {
            // Using TimeZoneInfo on dates far back towards the start of the christian calendar era poses problem. Don't do it if TZ = UTC
            if(timeZoneId == "UTC")
                return CreateDate(isoDateTime, "Utc");
            else
            {
                var tz = TimeZoneInfo.FindSystemTimeZoneById(timeZoneId);
                var dtun = CreateDate(isoDateTime);
                return TimeZoneInfo.ConvertTimeToUtc(dtun, tz);
            }
        }

        public static DateTime[] UtcDateForTimeZone(string[] isoDateTimes, string timeZoneId = "UTC")
        {
            return Array.ConvertAll(isoDateTimes, x => UtcDateForTimeZone(x, timeZoneId:timeZoneId));
        }

        public static DateTime TimeZoneToLocalDate(string isoDateTime, string timeZoneId = "UTC")
        {
            return UtcDateForTimeZone(isoDateTime, timeZoneId).ToLocalTime();
        }

        public static DateTime ConvertTime(string isoDateTime, string timeZoneId_from = "UTC", string timeZoneId_to = "UTC")
        {
            return TimeZoneInfo.ConvertTime(CreateDate(isoDateTime), 
                TimeZoneInfo.FindSystemTimeZoneById(timeZoneId_from), 
                TimeZoneInfo.FindSystemTimeZoneById(timeZoneId_to));
        }

        public static bool NumArrayEquals(double[] array)
        {
            var expected = CreateNumArray();
            return checkNumArrayEquals(array, expected);
        }

        public static bool NumArrayMissingValsEquals(double[] array)
        {
            var expected = CreateNumArray();
            expected[2] = double.NaN;
            return checkNumArrayEquals(array, expected);
        }

        private static bool checkNumArrayEquals(double[] array, double[] expected)
        {
            if (expected.Length != array.Length) return false;
            for (int i = 0; i < expected.Length; i++)
            {
                if (double.IsNaN(array[i]) && !double.IsNaN(expected[i]))
                    return false;
                if (!double.IsNaN(array[i]) && double.IsNaN(expected[i]))
                    return false;
                if (Math.Abs(array[i] - expected[i]) > 1e-10)
                    // there ARE rounding artefacts  even in an R statement sush as 1:5*1.1
                    return false;
            }
            return true;
        }

        public static double[] CreateNumArray()
        {
            return new[] { 1.1, 2.2, 3.3, 4.4, 5.5 };
        }

        public static double[] CreateNumArrayMissingVal()
        {
            var result = CreateNumArray();
            result[2] = double.NaN;
            return result;
        }

        private static DateTime[] CreateDailySequence(int numDays, DateTime startDate)
        {
            DateTime[] result = new DateTime[numDays];
            for (int i = 0; i < result.Length; i++)
                result[i] = startDate.AddDays(i);
            return result;
        }

        public static DateTime[] CreateDateArraySeconds(string isoDateTimeStart, int seconds)
        {
            var startDate = CreateDate(isoDateTimeStart);
            DateTime[] result = new DateTime[seconds];
            for (int i = 0; i < result.Length; i++)
                result[i] = startDate.AddSeconds(i);
            return result;
        }

        public static bool CheckIsDailySequence(DateTime[] values, string isoDateTimeStart, int numDays)
        {
            if (values.Length != numDays) return false;
            var startDate = CreateDate(isoDateTimeStart);
            DateTime[] expected = CreateDailySequence(numDays, startDate);
            for (int i = 0; i < expected.Length; i++)
                if (values[i] != expected[i]) return false;
            return true;
        }

        public static TimeSpan[] CreateTimeSpanArray(double secondsFirst, int bySecondIncrements)
        {
            var result = new TimeSpan[bySecondIncrements];
            for (int i = 0; i < result.Length; i++)
                result[i] = TimeSpan.FromSeconds(secondsFirst+ i*bySecondIncrements);
            return result;
        }

        public static bool TimeSpanEquals(TimeSpan tspan, string tspanString)
        {
            var expected = TimeSpan.Parse(tspanString, CultureInfo.InvariantCulture);
            return tspan.Equals(expected);
        }

        public static int[] CreateIntArray(int n)
        {
            var result = new int[n];
            for (int i = 0; i < result.Length; i++)
                result[i] = i;
            return result;
        }

        public static float[] CreateFloatArray()
        {
            return Array.ConvertAll(CreateNumArray(), x => (float)x);
        }

        public static float[][] CreateJaggedFloatArray()
        {
            var result = new float[3][];
            for (int i = 0; i < result.Length; i++)
            {
                var a = result[i] = new float[5];
                for (int j = 0; j < a.Length; j++)
                {
                    result[i][j] = 1.0f + a.Length * i + j;
                }
            }
            return result;
        }

        public static double[][] CreateJaggedDoubleArray()
        {
            return CreateJaggedFloatArray().ToDouble();
        }

        public static double[,] CreateRectDoubleArray()
        {
            return CreateJaggedFloatArray().ToDoubleRect();
        }

        public static float[,] CreateRectFloatArray()
        {
            return CreateJaggedFloatArray().ToFloatRect();
        }

        public static bool NumericMatrixEquals(double[,] matrix)
        {
            var expected = CreateJaggedFloatArray().ToDoubleRect();
            if (expected.GetLength(0) != matrix.GetLength(0))
            {
                Console.WriteLine("dim 0 different lengths");
                return false;
            }
            if (expected.GetLength(1) != matrix.GetLength(1))
            {
                Console.WriteLine("dim 1 different lengths");
                return false;
            }
            for (int i = 0; i < expected.GetLength(0); i++)
            {
                for (int j = 0; j < expected.GetLength(1); j++)
                {
                    if (Math.Abs(matrix[i, j] - expected[i, j]) > 1e-10)
                    {
                        //Console.WriteLine("Element index {0} is '{1}'", i, array[i]);
                        return false;
                    }
                }
            }
            return true;
        }

        public static bool StringArrayEquals(string[] array)
        {
            var expected = CreateStringArray();
            return checkArrayElements(array, expected);
        }

        public static bool StringArrayMissingValsEquals(string[] array)
        {
            var expected = CreateStringArray();
            expected[2] = null;
            return checkArrayElements(array, expected);
        }

        private static bool checkArrayElements(string[] array, string[] expected)
        {
            if (expected.Length != array.Length)
            {
                Console.WriteLine("array of strings: different lengths");
                return false;
            }
            for (int i = 0; i < expected.Length; i++)
            {
                if (array[i] != expected[i])
                {
                    Console.WriteLine("Element index {0} is '{1}'", i, array[i]);
                    return false;
                }
            }
            return true;
        }

        public static Dictionary<string, string> CreateStringDictionary()
        {
            return new Dictionary<string, string> { { "a", "A" }, { "b", "B" } };
        }

        public static Dictionary<string, object> CreateObjectDictionary()
        {
            return new Dictionary<string, object>
                {
                    {"a", new ArbitraryObject("A")},
                    {"b", "B"},
                    {"c", 123}
                };
        }

        public static void ThrowException(int stackDepth)
        {
            stackDepth = Math.Max(1, Math.Min(stackDepth, 100));
            if (stackDepth == 1)
                throw new Exception("An exception designed with a particular stack trace length");
            else
                ThrowException(stackDepth-1);
        }

        public static string GetExceptionMessage()
        {
            return ClrFacade.FormatException(CreateInnerExceptions());
        }

        public static Exception CreateInnerExceptions()
        {
            Exception exception = null;
            for (int i = 2; i >= 0; i--)
                exception = CreateException(i, exception);
            return exception;
        }

        public static Exception CreateException(int depth, Exception innerException)
        {
            return new Exception("Depth: " + depth.ToString(), innerException);
        }

		public static TestEnum GetTestEnum(string name)
		{
			TestEnum result;
			TestEnum.TryParse<TestEnum>(name, out result);
			return result;
		}

        private class ArbitraryObject
        {
            public string value { get; private set; }

            public ArbitraryObject(string value)
            {
                this.value = value;
            }
        }

        public static string[] CreateStringArray()
        {
            return new string[] { "ab", "bc", "cd", "de", "ef", };
        }

        public static double CreateDouble()
        {
            return 123.0;
        }

        public static bool DoubleEquals(double value)
        {
            return (123.0 == value);
        }

        public static float CreateFloat()
        {
            return 123.0f;
        }

        public static long CreateLong()
        {
            return 123;
        }

        public static int CreateInt()
        {
            return 123;
        }

        public static bool IntEquals(int value)
        {
            return (123 == value);
        }

        public static string CreateString()
        {
            return "ab";
        }

        public static bool StringEquals(string value)
        {
            return ("ab" == value);
        }

        private class MemTestObject
        {
            public string Text { get; set; }
            public static int counter = 0;
            public MemTestObject() { counter++; }
            ~MemTestObject() { counter--; }
        }

        public static int GetMemTestObjCounter()
        {
            return MemTestObject.counter;
        }

        public static object CreateMemTestObj()
        {
            return new MemTestObject();
        }

        /// <summary>
        /// This function takes time. Only call it when you need to.
        /// </summary>
        public static void CallGC()
        {
            GC.Collect(GC.MaxGeneration, GCCollectionMode.Forced);
            GC.WaitForPendingFinalizers();
            GC.GetTotalMemory(true);
        }

        public static TestObject CreateTestObject()
        {
            return new TestObject();
        }
    }

	public enum TestEnum
	{
		A,
		B,
		C
	}
	[Flags] public enum TestFlagEnum
	{
		None = 0,
		A = 1,
		B = 2,
		C = 4
	}

    public class TestObject
    {
        // Helps test constructors
        public TestObject()
        {

        }

        public TestObject(int f1)
        {
            this.FieldIntegerOne = f1;
        }

        public TestObject(int f1, int f2)
            : this(f1)
        {
            this.FieldIntegerTwo = f2;
        }

        public TestObject(double d1)
        {
            this.FieldDoubleOne = d1;
        }

        public TestObject(double d1, double d2)
            : this(d1)
        {
            this.FieldDoubleTwo = d2;
        }

        public TestObject(int f1, int f2, double d1, double d2)
        {
            FieldIntegerOne = f1;
            FieldIntegerTwo = f2;
            FieldDoubleOne = d1;
            FieldDoubleTwo = d2;
        }

        public int PublicInt;
#pragma warning disable 169
        private int PrivateInt;
#pragma warning restore 169
        public int GetPublicInt() { return PublicInt; }

        public int FieldIntegerOne;
        public int FieldIntegerTwo;

        public double FieldDoubleOne;
        public double FieldDoubleTwo;

        public int GetFieldIntegerOne() { return FieldIntegerOne; }
        public int GetFieldIntegerTwo() { return FieldIntegerTwo; }

        public int GetMethodWithParameters(int pOne, string pTwo) { return FieldIntegerOne; }

        public int PropertyIntegerOne { get; set; }
        public int PropertyIntegerTwo { get; set; }

        public static int StaticPublicInt;
#pragma warning disable 169
        private static int StaticPrivateInt;
#pragma warning restore 169
        public static int StaticGetPublicInt() { return StaticPublicInt; }

        public static int StaticFieldIntegerOne;
        public static int StaticFieldIntegerTwo;

        public static int StaticGetFieldIntegerOne() { return StaticFieldIntegerOne; }
        public static int StaticGetFieldIntegerTwo() { return StaticFieldIntegerTwo; }

        public static int StaticGetMethodWithParameters(int pOne, string pTwo) { return StaticFieldIntegerOne; }

        public static int StaticPropertyIntegerOne { get; set; }
        public static int StaticPropertyIntegerTwo { get; set; }

    }


    //public interface ITestObject
    //{
    //    int GetCurrentCounter();
    //}
    //public class TestObject : ITestObject
    //{
    //    /// <summary>
    //    /// This function takes time. Only call it when you need to.
    //    /// </summary>
    //    public void ClearAllAvailableMemory()
    //    {
    //        GC.Collect(GC.MaxGeneration, GCCollectionMode.Forced);
    //        GC.WaitForPendingFinalizers();
    //        GC.GetTotalMemory(true);
    //    }
    //    private static int counter = 0;
    //    public TestObject()
    //    {
    //        counter++;
    //    }
    //    private double[] blah = new double[2000000];
    //    ~TestObject()
    //    {
    //        counter--;
    //    }

    //    public int GetCurrentCounter()
    //    {
    //        return counter;
    //    }

    //    public int GetStringLen(string str)
    //    {
    //        return str.Length;
    //    }

    //    public string Clone(string str)
    //    {
    //        return str;
    //    }

    //    public static ITestObject GetInterface()
    //    {
    //        return new TestObject();
    //    }

    //    public static TestObject GetClass()
    //    {
    //        return new TestObject();
    //    }
    //}

}
