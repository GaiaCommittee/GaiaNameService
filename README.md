# Gaia Name Service

This services package provide name resolving service client.

## Introduction

Name resolving service is a service can map service name to service address,
thus services can use name to locate desired services, 
which can unbound the services dependency relationship.

## Functions

Gaia Name Service depends on "name" to "address" mapping, 
and provides name query, address query and name register functions.

A name has an expiration time, which needs to be update to keep it valid.
This mechanism can prove the corresponding service is still alive.
The standard expiration time is 3 seconds, 
and the automatic update thread will update the bound name every 1 second.

## Name and Address

Names are stored as Redis global keys with the prefix "gaia.names/".
This may add to the overload of the Redis server,
but this design can easily implement the expiration time mechanism.
According to its implementation method,
too many names are not recommended.

Addresses are in the format of text, which can satisfy the needs of any protocols.
Pay attention to that the address may be empty, 
and this depends on whether the service exposes a port to provide service or just use Gaia microservice middlewares.

If the return value of directly invoking the function to get the address text of a name is empty,
then the name may not exist, or its value is exactly empty.
Usually, those two situations are the same to handle if the user expect to get a non-empty address of the desired service.