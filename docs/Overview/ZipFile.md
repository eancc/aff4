---
layout: docs
title: The AFF4 ZipFile Volume
order: 3
---

The AFF4 *ZipFile* volume uses a zip file to store AFF4 objects. The AFF4 volume
has a unique URN which is also stored as a Zip file comment.

The AFF4 information model relating to the AFF4 objects stored in the ZipFile is
serialized into a zip member called *information.turtle* using the turtle RDF
serialization format.

### An AFF4 *ZipFileSegment*

A *ZipFileSegment* is an AFF4 object representing a single member of the zip
file.

#### ZipFileSegment filename encoding.

Like other AFF4 objects, the Segment has a fully qualified URN. This URN
is encoded as the zip member file name using the following encoding scheme:

1. If the Segment URN begins with the Volume URN, the filename consists of the
non common prefix. Therefore filenames are essentially specified relative to the
volume URN.

2. The following characters must be % encoded: \/:*?"<>|

3. Repeated sequences of / must be % encoded. Single / sequences should be
preserved.

4. Filenames should not contain directory traversal characters such as . or .. -
They should also not begin with a leading /.

The following example show some URNs encoded into a segment filename. For this example we assume that the Volume URN is *aff4://21803da1-a4e4-4cf7-b866-f4f94c6fb041*:

| URN                                                | Zip filename  |
|----------------------------------------------------|---------------|
| aff4://21803da1-a4e4-4cf7-b866-f4f94c6fb041/bash   | bash          |
| aff4://7a71a6a-4d33-4d96-ad99-5cda758c3a19/image.dd| aff4%3a%2f%2f7a71a6a-4d33-4d96-ad99-5cda758c3a19/image.dd

Although the implementation can choose any URNs to describe objects that it
creates (as long as they are globally unique), usually implementations will
select a URN which is derived from the volume URN so that the segment filenames
are short and meaningful.

For example consider the following example where a logical image of a file is
taken:

```sh
$ aff4imager -i /bin/ls -o /tmp/test.aff4
Adding /bin/ls

$ unzip -l /tmp/test.aff4
Archive:  /tmp/test.aff4
aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7
  Length      Date    Time    Name
---------  ---------- -----   ----
     1639  2015-02-24 12:39   information.yaml
      413  2015-02-24 12:39   information.turtle
    51005  2015-02-24 12:39   bin/ls/00000000
       16  2015-02-24 12:39   bin/ls/00000000/index
---------                     -------
    53073                     4 files
```

In this example, the imager acquires the file */bin/ls* into a new volume. The
new volume receives the URN *aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7* (and
it is stored in */tmp/test.aff4*). The new image URN is selected to be
*aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7/bin/ls* - this is still unique but
it provides some context.

We can use a regular zip file tool to examine the members of the zip file. We
can see the segment filename *bin/ls/00000000*. As far as the AFF4 information
model is concerned, this segment represents the AFF4 object with URN
*aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7/bin/ls/00000000*.

One advantage of the AFF4 ZipFile volume is that, since it uses the well tested
and ubiquotous zip file format there is widespread support for recovering
corrupted zip files, carving zip files from disk and manipulating zip
file. Another advantage is that a ZipFile volume can be trivially extracted to a
directory, which turns it into an AFF4 *DirectoryVolume*.

Additionally the ZipFile supports amending the archive and adding additional
segments. AFF4 ZipFile volumes are therefore maliable. It is possible to add new
AFF4 objects into the same volume. This is useful when acquiring new logical
files in several steps - or acquiring a disk image and a memory image at the
same time.

## Information model

| Predicate |  Description |
|-----------|--------------|
| *http://aff4.org/Schema#stored* | When applied to a ZipFile volume, this specifies the URN of the stream which contains the ZipFile volume (This fact is typically inferred). When applied to a ZipFileSegment this specifies the ZipFile volume which contains the segment. |
| *http://www.w3.org/1999/02/22-rdf-syntax-ns#type* | The type of object. For ZipFile object this will be the URN *http://aff4.org/Schema#zip_volume* . This fact is also typically inferred when a zip file format is detected. |
| *http://www.w3.org/1999/02/22-rdf-syntax-ns#type* | The type of object. For ZipFileSegments objects this will be the URN *http://aff4.org/Schema#zip_segment* . This fact is typically inferred when a zip file format is parsed. |

### Example

The following is the information model contained in the ZipFile volume obtained
above:

```sh
$ aff4imager -V /tmp/test.aff4
@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .
@prefix aff4: <http://aff4.org/Schema#> .
@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .

<aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7/bin/ls>
    aff4:chunk_size 32768 ;
    aff4:chunks_per_segment 1024 ;
    aff4:compression <https://www.ietf.org/rfc/rfc1950.txt> ;
    aff4:size 110080 ;
    aff4:stored <aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7> ;
    a aff4:image .

$ aff4imager -v -V /tmp/test.aff4
@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .
@prefix aff4: <http://aff4.org/Schema#> .
@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .

<aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7>
    aff4:stored <file:///tmp/test.aff4> ;
    a aff4:zip_volume .

<aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7/bin/ls>
    aff4:chunk_size 32768 ;
    aff4:chunks_per_segment 1024 ;
    aff4:compression <https://www.ietf.org/rfc/rfc1950.txt> ;
    aff4:size 110080 ;
    aff4:stored <aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7> ;
    a aff4:image .

<aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7/bin/ls/00000000>
    aff4:stored <aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7> ;
    a aff4:zip_segment .

<aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7/bin/ls/00000000/index>
    aff4:stored <aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7> ;
    a aff4:zip_segment .

<aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7/information.turtle>
    aff4:stored <aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7> ;
    a aff4:zip_segment .

<aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7/information.yaml>
    aff4:stored <aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7> ;
    a aff4:zip_segment .
```

In the first example we dump the information explicitely stored in the
volume. We see a single stream
*aff4://6cb5ce35-b28c-481a-8f0b-a79f17a32dd7/bin/ls* which is of type
*http://aff4.org/Schema#image*.

In the second example we request (using the -v) to dump all triples, even
inferred ones. In this second case we see many additional objects, such as the
volume itself (*aff4:zip_volume*), and each ZipFileSegment (*aff4:zip_segment*)
contained in the volume. These objects are known by the AFF4 Resolver by parsing
the Zip archive itself, hence they do not need to be specified in the
*information.turtle* file itself.