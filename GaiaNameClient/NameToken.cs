using System;
using System.Threading;
using StackExchange.Redis;

namespace Gaia.NameService
{
    /// <summary>
    /// Name token represents a name resolving item, which contains a name and an address of a service.
    /// </summary>
    public class NameToken : IDisposable
    {
        /// <summary>
        /// Redis database instance.
        /// </summary>
        private readonly IDatabase Database = null;
        /// <summary>
        /// Redis server instance.
        /// </summary>
        private readonly IServer Server = null;

        /// <summary>
        /// Name of this name token.
        /// </summary>
        public readonly string Name;
        
        /// <summary>
        /// Address text property.
        /// </summary>
        public string Address
        {
            get => GetAddress();
            set => SetAddress(value);
        }

        /// <summary>
        /// Constructor, should be invoked by NameClient.
        /// </summary>
        /// <param name="server">Redis server instance.</param>
        /// <param name="database">Redis database instance</param>
        /// <param name="name">Name of this name token.</param>
        /// <param name="address">Address of this name token.</param>
        internal NameToken(IServer server, IDatabase database, string name, string address = "")
        {
            Name = name;
            Server = server;
            Database = database;

            Database?.StringSet($"names/{Name}", address);
            Database?.KeyExpire($"names/{Name}", TimeSpan.FromSeconds(3));
        }

        /// <summary>
        /// Unregister the name and stop the background updater.
        /// </summary>
        public void Dispose()
        {
            StopBackgroundUpdater();
            Database?.KeyDelete($"names/{Name}");
        }

        /// <summary>
        /// Update the timestamp to keep it valid.
        /// </summary>
        public void Update()
        {
            Database?.KeyExpire($"names/{Name}", TimeSpan.FromSeconds(3));
        }

        /// <summary>
        /// Get the address text of the corresponding name.
        /// </summary>
        /// <returns>Address text of the corresponding name.</returns>
        private string GetAddress()
        {
            var result = Database?.StringGet($"names/{Name}");
            if (result != null)
            {
                return result;
            }

            return "";
        }
        
        /// <summary>
        /// Set the address text of the corresponding name.
        /// </summary>
        private void SetAddress(string address)
        {
            Database?.StringSet($"names/{Name}", address);
        }

        /// <summary>
        /// Life flag for background updater thread.
        /// Updater will exit in the frame when this flag is false.
        /// </summary>
        private bool UpdaterFlag = false;
        /// <summary>
        /// Background updater thread.
        /// </summary>
        private Thread UpdaterThread = null;
        
        /// <summary>
        /// Start the background auto update thread.
        /// That thread will auto update this name every second.
        /// </summary>
        public void StartBackgroundUpdater()
        {
            if (UpdaterFlag) return;
            
            UpdaterFlag = true;
            UpdaterThread = new Thread( delegate()
            {
                while (UpdaterFlag)
                {
                    Update();
                    Thread.Sleep(TimeSpan.FromSeconds(1));
                }
            });
            UpdaterThread.Start();
        }

        /// <summary>
        /// Stop the background auto update thread.
        /// </summary>
        public void StopBackgroundUpdater()
        {
            if (!UpdaterFlag) return;
            UpdaterFlag = false;
            UpdaterThread.Join();
        }
    }
}