using System;
using System.Diagnostics;

namespace Rclr
{
    /// <summary>
    /// A class to use in order to measure the time cost of rClr features.
    /// </summary>
    public class PerformanceProfiling
    {
        public static double GetElapsedSeconds(Stopwatch sw)
        {
            long nanosecPerTick = (1000L * 1000L * 1000L) / Stopwatch.Frequency;
            var deltaNano = sw.ElapsedTicks * nanosecPerTick;
            return deltaNano / 1e9;
        }

        public void SetDoubleArrays(int seed, int length, int numArrays)
        {
            doubleArray = new double[numArrays][];
            Random r = new Random(seed);
            for (int i = 0; i < doubleArray.Length; i++)
            {
                doubleArray[i] = new double[length];
                for (int j = 0; j < length; j++)
                    doubleArray[i][j] = r.NextDouble();
            }
            counterDouble = 0;
        }

        private double[][] doubleArray;
        private int counterDouble;
        public double[] GetNextArrayDouble()
        {
            var res = doubleArray[counterDouble % doubleArray.Length];
            counterDouble++;
            return res;
        }

        public void CallMethodWithArrayDouble(double[] someParameter)
        {
            // nothing
        }

        public void ArrayDoubleSink(double[] ignored)
        {
            // nothing.
        }

        public void DoNothing()
        {
            // nothing.
        }
    }
}
