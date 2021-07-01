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
        /// Construct and connect to a Redis server.
        /// </summary>
        /// <param name="port">Port of the Redis server.</param>
        /// <param name="ip">IP address of the Redis server.</param>
        public NameClient(uint port = 6379, string ip = "127.0.0.1")
        {
            var connection = ConnectionMultiplexer.Connect($"{ip}:{port.ToString()}");
            if (!connection.IsConnected) return;
            
            Server = connection.GetServer(connection.GetEndPoints()[0]);
            Database = connection.GetDatabase();
        }

        /// <summary>
        /// Query all registered name.
        /// </summary>
        /// <remarks>It is a time consuming function.</remarks>
        /// <returns>All valid registered names with out prefix "gaia.names/".</returns>
        public IEnumerable<string> GetNames()
        {
            return Server.Keys(pattern: "gaia.names/*").Select(name => ((string) name).Substring(11));
        }

        /// <summary>
        /// Query whether a name exists or not.
        /// </summary>
        /// <param name="name">Name to query.</param>
        /// <returns>Whether this name exists or not.</returns>
        public bool HasName(string name)
        {
            return Database.KeyExists($"gaia.names/{name}");
        }

        /// <summary>
        /// Get the address text of the given name.
        /// </summary>
        /// <param name="name">The name to query.</param>
        /// <returns>Address text of the given name, or empty if it does not exist.</returns>
        public string GetAddress(string name)
        {
            return Database.StringGet($"gaia.names/{name}");
        }

        /// <summary>
        /// Construct a name token and register the given name.
        /// </summary>
        /// <param name="name">The name to hold.</param>
        /// <param name="address">The address of this name.</param>
        /// <returns>Corresponding name token.</returns>
        public NameToken HoldName(string name, string address = "")
        {
            return new(Server, Database, name, address);
        }
    }
}