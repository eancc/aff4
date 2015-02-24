---
layout: docs
title: The AFF4 Map stream.
order: 4
---

A lot of work in digital forensics involves copying data around. For example,
carving files usually results with the carved files copied out of the image for
testing. If you image a RAID array separately you end up with 3-5 disk images
and typically you will need to copy them into a logical image (unless your
favourite software supports RAID reconstruction). When you copy a file out of
the image using sleuthkit, you are actually copying bits of data directly from
the image - either allocated clusters or from within the MFT entry (for resident
files).

All these copies are wasteful of disk space. They are also hard to manage
because pretty soon you end up with lots of copies of the same data in different
ways. There must be a better way!!!

Now there is. By having the underlying forensic format doing all the mapping
itself it is possible to use tools which are not capable of doing these
transformations by themselves. This is all about tool reuse. For example suppose
you have a carver which is used to work on dd images. But you want to use it on
the virtual memory image of the firefox process. In the past you had to copy the
virtual memory out (it could be 2-4gb) then run the carver on it, and possibly
end up with about 3 or 4 copies of the same data - for each process address
space!!!

Its much easier to have a tool such as Rekall create the initial maps for each
process (with zero storage overheads), and then carvers can just use the maps
without understanding anything about memory forensics. In this way the AFF4
format is more of an interchange format - allowing tools to be used on the
results from other tools.

The map stream is an AFF4 object which specifies some mapping between stream
offsets and other streams (termed *target* streams). The map specifies a
transformation between offsets in the map's space and other offsets in one or
more targets. This is illustrated below:

![The AFF4 map overview](/img/aff4_map.png "The AFF4 Map overview")

The map specifies a set of offset ranges and the corresponding ranges in the
backing stream. As the figure shows this allows the map to present a sparse view
of the data (i.e. reads in the sparse region will return null padded data).

### Encoding of the map

The AFF4 Map object stores the map in a binary format in a segment with a URN
obtained by appending "map" to its URN. The segment contains a list of records:

```c++
struct Range {
  uint64_t map_offset;
  uint64_t target_offset;
  uint64_t length;
  uint32_t target_id;
}__attribute__((packed));
```

Where targets is an index into the list of targets specified in the target
index. The target index, in turn is another stream with a URN obtained by
appending "idx" to the map stream's URN. The target index contains all the
targets separated by carriage returns - one per line.

### Information model

| Predicate |  Description |
|-----------|--------------|
| *http://www.w3.org/1999/02/22-rdf-syntax-ns#type* | The type of object. For AFF4 Map objects this will be the URN *http://aff4.org/Schema#map* . |

### AFF4 Map properties

The AFF4 map object can be written in random sparse order - i.e. the stream can
be seeked as it is being written. This causes the object to automatically build
the map while writing the data contiguously to the backing stream. Typically an
AFF4 Map stream will use an AFF4 Image stream as its backing store.

Here is an example of two ranges being created directly on the map object:

```c++
    // Now an image is created inside the volume.
    AFF4ScopedPtr<AFF4Map> image = AFF4Map::NewAFF4Map(
        &resolver, zip->urn.Append(image_name), volume_urn);

    // Maps are written in random order.
    image->Seek(50, SEEK_SET);
    image->Write("50 - This is the position.");

    image->Seek(0, SEEK_SET);
    image->Write("00 - This is the position.");
```

Let us examine the resulting volume:

```sh
$ unzip -l /tmp/aff4_test.zip
Archive:  /tmp/aff4_test.zip
aff4://37a71a6a-4d33-4d96-ad99-5cda758c3a19
  Length      Date    Time    Name
---------  ---------- -----   ----
     2345  2015-02-23 16:59   information.yaml
      467  2015-02-23 16:59   information.turtle
       84  2015-02-23 16:59   image.dd/map
       58  2015-02-23 16:59   image.dd/idx
       38  2015-02-23 16:59   image.dd/data/00000000
        4  2015-02-23 16:59   image.dd/data/00000000/index
---------                     -------
     2996                     6 files

$ aff4imager -V /tmp/aff4_test.zip
@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .
@prefix aff4: <http://aff4.org/Schema#> .
@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .

<aff4://37a71a6a-4d33-4d96-ad99-5cda758c3a19/image.dd>
    a aff4:map .

<aff4://37a71a6a-4d33-4d96-ad99-5cda758c3a19/image.dd/data>
    aff4:chunk_size 32768 ;
    aff4:chunks_per_segment 1024 ;
    aff4:compression <https://www.ietf.org/rfc/rfc1950.txt> ;
    aff4:size 54 ;
    aff4:stored <aff4://37a71a6a-4d33-4d96-ad99-5cda758c3a19> ;
    a aff4:image .
```

We can see the map stream's "map" stream and "idx" stream and also the
underlying target stream, in this case
*aff4://37a71a6a-4d33-4d96-ad99-5cda758c3a19/image.dd/data*.
