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
                if (_knfOnlineFeature.impl != IntPtr.Zero)
                {
                    _knfOnlineFeature.impl = IntPtr.Zero;
                }
                this._disposed = true;
            }
        }

        ~OnlineBase()
        {
            Dispose(this._disposed);
        }
        internal IntPtr _opts = IntPtr.Zero;
        internal KnfOnlineFeature _knfOnlineFeature;
        internal bool _disposed = false;
    }
}
