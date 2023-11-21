using KaldiNativeFbankSharp.Struct;

namespace KaldiNativeFbankSharp
{
    public class OnlineBase : IDisposable
    {
        public void Dispose()
        {
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposing)
            {
                if (_opts != IntPtr.Zero)
                {
                    _opts = IntPtr.Zero;
                }
                if (_knfOnlineFbank.impl != IntPtr.Zero)
                {
                    _knfOnlineFbank.impl = IntPtr.Zero;
                }
                this._disposed = true;
            }
        }

        ~OnlineBase()
        {
            Dispose(this._disposed);
        }
        internal IntPtr _opts = IntPtr.Zero;
        internal KnfOnlineFbank _knfOnlineFbank;
        internal bool _disposed = false;
    }
}
