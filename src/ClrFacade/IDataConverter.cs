using System;
using System.Collections.Generic;
using System.Text;
using RDotNet;

namespace Rclr
{
    /// <summary>
    /// Interface for objects that are convering CLR objects to a representation in R
    /// </summary>
    /// <remarks>Currently the only concrete implementation is a data converter that uses RDotNet 
    /// to expose CLR objects to R</remarks>
    public interface IDataConverter
    {
        object ConvertToR(object obj);
        object ConvertFromR(IntPtr pointer, int sexptype);

        /// <summary>
        /// Return a reference to the object currently handled by the custom data converter, if any is in use.
        /// </summary>
        /// <remarks>
        /// See https://rclr.codeplex.com/workitem/33 for why we need this.
        /// </remarks>
        object CurrentObject { get; }

        // TODO: this should not be here, but for now a convenient way to access Rf_error via R.NET
        void Error(string msg);

        SymbolicExpression CreateSymbolicExpression(IntPtr sexp);

        object[] ConvertSymbolicExpressions(object[] arguments);

        object ConvertSymbolicExpression(object obj);
    }
}
