using System;
using System.Collections.Generic;
using System.Linq;
using StackExchange.Redis;

namespace Gaia.NameService
{
    /// <summary>
    /// Client for name service.
    /// This client provides name query and register services.
    /// </summary>
    public class NameClient
    {
        /// <summary>
        /// Redis server instance.
        /// </summary>
        private readonly IServer Server = null;
        /// <summary>
        /// Redis database instance.
        /// </summary>
        private readonly IDatabase Database = null;

        /// <summary>
        /// Names to update.
        /// Those names are all registered from this client,
        /// and will be automatically unregistered when this client destruct.
        /// </summary>
        private readonly HashSet<string> Names = new HashSet<string>();

        /// <summary>
        /// Construct and connect to a Redis server.
        /// </summary>
        /// <param name="port">Port of the Redis server.</param>
        /// <param name="ip">IP address of the Redis server.</param>
        public NameClient(uint port = 6379, string ip = "127.0.0.1") :
            this(ConnectionMultiplexer.Connect($"{ip}:{port.ToString()}"))
        {}

        /// <summary>
        /// Reuse the connection to a Redis server.
        /// </summary>
        /// <param name="connection"></param>
        public NameClient(IConnectionMultiplexer connection)
        {
            Server = connection.GetServer(connection.GetEndPoints()[0]);
            Database = connection.GetDatabase();
        }

        /// <summary>
        /// Query all registered name.
        /// </summary>
        /// <remarks>It is a time consuming function.</remarks>
        /// <returns>All valid registered names with out prefix "names/".</returns>
        public IEnumerable<string> GetNames()
        {
            return Server.Keys(pattern: "names/*").Select(name => ((string) name).Split("/")[^1]);
        }

        /// <summary>
        /// Query whether a name exists or not.
        /// </summary>
        /// <param name="name">Name to query.</param>
        /// <returns>Whether this name exists or not.</returns>
        public bool HasName(string name)
        {
            return Database.KeyExists($"names/{name}");
        }

        /// <summary>
        /// Get the address text of the given name.
        /// </summary>
        /// <param name="name">The name to query.</param>
        /// <returns>Address text of the given name, or empty if it does not exist.</returns>
        public string GetAddress(string name)
        {
            return Database.StringGet($"names/{name}");
        }

        /// <summary>
        /// Update all names in the update list.
        /// This function call will update the remaining expiration of names to 3 seconds.
        /// </summary>
        public void Update()
        {
            foreach (var name in Names)
            {
                Database.KeyExpire($"names/{name}", TimeSpan.FromSeconds(3));
            }
        }

        /// <summary>
        /// Register a name and add it to the update list.
        /// </summary>
        /// <param name="name">Name to register.</param>
        /// <param name="address">Address corresponding to this name.</param>
        public void RegisterName(string name, string address = "")
        {
            Database.StringSet($"names/{name}", address);
            Database.KeyExpire($"names/{name}", TimeSpan.FromSeconds(3));
            Names.Add(name);
        }

        /// <summary>
        /// Unregister a name and remove it from the update list.
        /// </summary>
        /// <param name="name">Name to unregister.</param>
        public void UnregisterName(string name)
        {
            Database.KeyDelete($"names/{name}");
            Names.Remove($"names/{name}");
        }
    }
}