---
layout: docs
title: File Based Streams
order: 1
---

AFF4 uses *FileBackedObject* to store data backed by a file on the
filesystem. Like all AFF4 objects, the *FileBackedObject* has a URN which uses
the *file* scheme to denote the path to the file.

Strictly the file URN is not globally unique, and therefore not suitable as an
AFF4 URN. For this reason it is not usually stored permanently in volumes, but
it is usually inferred.

The AFF4 library will not usually create a new file or overwrite an existing
file. Unless the attribute *http://aff4.org/VolatileSchema#writable* is set to
"truncate" or "append". This mechanism requires users of the library to specify
in advance which files will be created or overwritten by setting this attribute.

### Information Model

| Predicate |  Description |
|-----------|--------------|
| *http://aff4.org/VolatileSchema#writable* | This attribute signals that the file object should be writable. It can take on the value "truncate" or "append". Note that this predicate is volatile which means it is never written into an AFF4 container. |
