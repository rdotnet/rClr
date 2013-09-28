namespace Rclr
{
    /// <summary>
    /// Extension methods to facilitate data marshalling between R and the CLR
    /// </summary>
    public static class DataConverterExtensions
    {
        public static double[][] ToDouble(this float[][] array)
        {
            var result = new double[array.Length][];
            for (int i = 0; i < array.Length; i++)
            {
                result[i] = new double[array[i].Length];
                for (int j = 0; j < array[i].Length; j++)
                    result[i][j] = array[i][j];
            }
            return result;
        }

        public static double[,] ToDoubleRect(this float[][] array)
        {
            return array.ToDouble().ToDoubleRect();
        }

        public static double[,] ToDoubleRect(this double[][] array)
        {
            var result = new double[array.Length, array[0].Length];
            for (int i = 0; i < array.Length; i++)
                for (int j = 0; j < array[0].Length; j++)
                    result[i, j] = array[i][j];
            return result;
        }

        public static string[,] ToStringRect(this string[][] array)
        {
            var result = new string[array.Length, array[0].Length];
            for (int i = 0; i < array.Length; i++)
                for (int j = 0; j < array[0].Length; j++)
                    result[i, j] = array[i][j];
            return result;
        }

        public static double[,] ToDoubleRect(this float[,] array)
        {
            int n = array.GetLength(0);
            int m = array.GetLength(1);
            var result = new double[n, m];
            for (int i = 0; i < n; i++)
                for (int j = 0; j < m; j++)
                    result[i, j] = array[i, j];
            return result;
        }

        public static float[,] ToFloatRect(this float[][] array)
        {
            var result = new float[array.Length, array[0].Length];
            for (int i = 0; i < array.Length; i++)
                for (int j = 0; j < array[0].Length; j++)
                    result[i, j] = array[i][j];
            return result;
        }
    }
}