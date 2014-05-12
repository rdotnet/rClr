using System;
using System.Reflection;
using Rclr;
using System.Collections.Generic;
using Rclr.Tests.RefClasses;

namespace Rclr
{
    /// <summary>
    /// Do not modify the .cs file: T4 generated class to support the unit tests for method binding
    /// </summary>
    public class TestMethodBinding : ITestMethodBindings
    {
        public static string[] SomeStaticMethod(object obj) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(object obj) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(object obj) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(string x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(string x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(string x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(double x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(double x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(double x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(float x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(float x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(float x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(int x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(int x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(int x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(bool x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(bool x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(bool x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(DateTime x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(DateTime x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(DateTime x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(byte x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(byte x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(byte x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(char x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(char x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(char x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(object obj1, object obj2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(object obj1, object obj2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(object obj1, object obj2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(string x1, string x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(string x1, string x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(string x1, string x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(double x1, double x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(double x1, double x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(double x1, double x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(float x1, float x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(float x1, float x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(float x1, float x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(int x1, int x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(int x1, int x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(int x1, int x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(bool x1, bool x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(bool x1, bool x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(bool x1, bool x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(DateTime x1, DateTime x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(DateTime x1, DateTime x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(DateTime x1, DateTime x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(byte x1, byte x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(byte x1, byte x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(byte x1, byte x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(char x1, char x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(char x1, char x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(char x1, char x2) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(object[] obj) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(object[] obj) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(object[] obj) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(string[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(string[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(string[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(double[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(double[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(double[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(float[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(float[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(float[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(int[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(int[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(int[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(bool[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(bool[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(bool[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(DateTime[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(DateTime[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(DateTime[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(byte[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(byte[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(byte[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(char[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(char[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(char[] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(object[,] obj) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(object[,] obj) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(object[,] obj) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(string[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(string[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(string[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(double[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(double[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(double[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(float[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(float[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(float[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(int[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(int[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(int[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(bool[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(bool[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(bool[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(DateTime[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(DateTime[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(DateTime[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(byte[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(byte[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(byte[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(char[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(char[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(char[,] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(object[][] obj) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(object[][] obj) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(object[][] obj) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(string[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(string[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(string[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(double[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(double[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(double[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(float[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(float[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(float[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(int[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(int[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(int[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(bool[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(bool[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(bool[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(DateTime[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(DateTime[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(DateTime[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(byte[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(byte[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(byte[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(char[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(char[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(char[][] x) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(double x, string y) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(double x, string y) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(double x, string y) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(double x, string y, string z) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(double x, string y, string z) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(double x, string y, string z) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(double x, string[] y) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(double x, string[] y) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(double x, string[] y) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(string x, string[] y) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(string x, string[] y) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(string x, string[] y) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(string[] x, string y) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(string[] x, string y) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(string[] x, string y) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public static string[] SomeStaticMethod(string[] x, string[] y) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        public string[] SomeInstanceMethod(string[] x, string[] y) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }
        string[] ITestMethodBindings.SomeExplicitlyImplementedMethod(string[] x, string[] y) { return ReflectionHelper.GetMethodParameterTypes(MethodBase.GetCurrentMethod()); }


        public static string[] GetOptionalParamsTestCases() {
			var list = new List<string>();
			list.Add("int p1, int p2, params int [] p3");
			list.Add("object p1, int p2, params int [] p3");
			list.Add("int p1, object p2, params int [] p3");
			list.Add("object p1, object p2, params int [] p3");
			list.Add("int p1, int p2, params object [] p3");
			list.Add("object p1, int p2, params object [] p3");
			list.Add("int p1, object p2, params object [] p3");
			list.Add("object p1, object p2, params object [] p3");
			list.Add("object p1");
			list.Add("int p1");
			return list.ToArray();
		}
        public static string SomeMethodWithVarArgs(int p1, int p2, params int [] p3)
        {
            return "int p1, int p2, params int [] p3";
        }
        public static string SomeMethodWithVarArgs(object p1, int p2, params int [] p3)
        {
            return "object p1, int p2, params int [] p3";
        }
        public static string SomeMethodWithVarArgs(int p1, object p2, params int [] p3)
        {
            return "int p1, object p2, params int [] p3";
        }
        public static string SomeMethodWithVarArgs(object p1, object p2, params int [] p3)
        {
            return "object p1, object p2, params int [] p3";
        }
        public static string SomeMethodWithVarArgs(int p1, int p2, params object [] p3)
        {
            return "int p1, int p2, params object [] p3";
        }
        public static string SomeMethodWithVarArgs(object p1, int p2, params object [] p3)
        {
            return "object p1, int p2, params object [] p3";
        }
        public static string SomeMethodWithVarArgs(int p1, object p2, params object [] p3)
        {
            return "int p1, object p2, params object [] p3";
        }
        public static string SomeMethodWithVarArgs(object p1, object p2, params object [] p3)
        {
            return "object p1, object p2, params object [] p3";
        }
        public static string SomeMethodWithVarArgs(object p1)
        {
            return "object p1";
        }
        public static string SomeMethodWithVarArgs(int p1)
        {
            return "int p1";
        }

//========== Next section is to test the support (or lack thereof) with the most ambiguous cases with optional parameters

        public static string MultipleMatchVarArgs(object p1, InterfaceOne p2, params object[] p3)
        {
            return "InterfaceOne";
        }
        public static string MultipleMatchVarArgs(object p1, InterfaceTwo p2, params object[] p3)
        {
            return "InterfaceTwo";
        }
        public static string MultipleMatchVarArgs(object p1, InterfaceBaseOne p2, params object[] p3)
        {
            return "InterfaceBaseOne";
        }
        public static string MultipleMatchVarArgs(object p1, InterfaceBaseTwo p2, params object[] p3)
        {
            return "InterfaceBaseTwo";
        }
        public static string MultipleMatchVarArgs(object p1, BaseAbstractClassOne p2, params object[] p3)
        {
            return "BaseAbstractClassOne";
        }
        public static string MultipleMatchVarArgs(object p1, LevelOneClass p2, params object[] p3)
        {
            return "LevelOneClass";
        }
        public static string MultipleMatchVarArgs(object p1, LevelTwoClass p2, params object[] p3)
        {
            return "LevelTwoClass";
        }

    }

    public interface ITestMethodBindings
    {
        string[] SomeExplicitlyImplementedMethod(object obj);
        string[] SomeExplicitlyImplementedMethod(string x);
        string[] SomeExplicitlyImplementedMethod(double x);
        string[] SomeExplicitlyImplementedMethod(float x);
        string[] SomeExplicitlyImplementedMethod(int x);
        string[] SomeExplicitlyImplementedMethod(bool x);
        string[] SomeExplicitlyImplementedMethod(DateTime x);
        string[] SomeExplicitlyImplementedMethod(byte x);
        string[] SomeExplicitlyImplementedMethod(char x);
        string[] SomeExplicitlyImplementedMethod(object obj1, object obj2);
        string[] SomeExplicitlyImplementedMethod(string x1, string x2);
        string[] SomeExplicitlyImplementedMethod(double x1, double x2);
        string[] SomeExplicitlyImplementedMethod(float x1, float x2);
        string[] SomeExplicitlyImplementedMethod(int x1, int x2);
        string[] SomeExplicitlyImplementedMethod(bool x1, bool x2);
        string[] SomeExplicitlyImplementedMethod(DateTime x1, DateTime x2);
        string[] SomeExplicitlyImplementedMethod(byte x1, byte x2);
        string[] SomeExplicitlyImplementedMethod(char x1, char x2);
        string[] SomeExplicitlyImplementedMethod(object[] obj);
        string[] SomeExplicitlyImplementedMethod(string[] x);
        string[] SomeExplicitlyImplementedMethod(double[] x);
        string[] SomeExplicitlyImplementedMethod(float[] x);
        string[] SomeExplicitlyImplementedMethod(int[] x);
        string[] SomeExplicitlyImplementedMethod(bool[] x);
        string[] SomeExplicitlyImplementedMethod(DateTime[] x);
        string[] SomeExplicitlyImplementedMethod(byte[] x);
        string[] SomeExplicitlyImplementedMethod(char[] x);
        string[] SomeExplicitlyImplementedMethod(object[,] obj);
        string[] SomeExplicitlyImplementedMethod(string[,] x);
        string[] SomeExplicitlyImplementedMethod(double[,] x);
        string[] SomeExplicitlyImplementedMethod(float[,] x);
        string[] SomeExplicitlyImplementedMethod(int[,] x);
        string[] SomeExplicitlyImplementedMethod(bool[,] x);
        string[] SomeExplicitlyImplementedMethod(DateTime[,] x);
        string[] SomeExplicitlyImplementedMethod(byte[,] x);
        string[] SomeExplicitlyImplementedMethod(char[,] x);
        string[] SomeExplicitlyImplementedMethod(object[][] obj);
        string[] SomeExplicitlyImplementedMethod(string[][] x);
        string[] SomeExplicitlyImplementedMethod(double[][] x);
        string[] SomeExplicitlyImplementedMethod(float[][] x);
        string[] SomeExplicitlyImplementedMethod(int[][] x);
        string[] SomeExplicitlyImplementedMethod(bool[][] x);
        string[] SomeExplicitlyImplementedMethod(DateTime[][] x);
        string[] SomeExplicitlyImplementedMethod(byte[][] x);
        string[] SomeExplicitlyImplementedMethod(char[][] x);
        string[] SomeExplicitlyImplementedMethod(double x, string y);
        string[] SomeExplicitlyImplementedMethod(double x, string y, string z);
        string[] SomeExplicitlyImplementedMethod(double x, string[] y);
        string[] SomeExplicitlyImplementedMethod(string x, string[] y);
        string[] SomeExplicitlyImplementedMethod(string[] x, string y);
        string[] SomeExplicitlyImplementedMethod(string[] x, string[] y);
    }
}
