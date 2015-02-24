---
layout: docs
title: The AFF4 Image stream.
order: 3
---

The AFF4 Image stream is the workhorse of AFF4 forensic images. It is designed
to efficiently store a large quantity of compressed data while still making it
fast to access the data randomly.

The image data is divided in *chunks*. Each chunk is compressed separately. A
number of chunks are collected into a single *segment* termed a *bevy*. Each
bevy also has another segment called the *bevy index* which contains the start
offset of each compressed chunk inside the bevy.

The bevy URN is derived from the AFF4 Image URN by appending an 8 digit decimal,
zero padded bevy id (an incrementing integer started from 0). Each bevy's index
has a URN which is created by appending "/index" to the bevy's URN.

For example, for an Image with URN
*aff4://05d7e827-5a23-4c17-97d5-190542e05b92/dev/sda1*, the third bevy will be
named *aff4://05d7e827-5a23-4c17-97d5-190542e05b92/dev/sda1/00000002* and the
third bevy's index will be named
*aff4://05d7e827-5a23-4c17-97d5-190542e05b92/dev/sda1/00000002/index*.

![The AFF4 Image format overview](/img/aff4_image.png "The AFF4 Image format overview")

### AFF4 Image properties

AFF4 Image objects are suitable for writing large quantities of contiguous data
- for example a disk image. Note that when writing the stream, it is not
possible to seek in the stream and that AFF4 Image streams are not sparse. The
stream is also not encrypted or authenticated.

AFF4 Image implementations may cache some of the chunks to avoid decompressing
frequently accessed chunks.

### Information model

Note that an AFF4 Image stream does not need to be stored in a single volume -
it may be split across multiple volumes. The AFF4 Image does not actually use
the *http://aff4.org/Schema#stored* predicate at all. The resolver attempts to
locate the segments which make up the AFF4 Image, and therefore only needs to
know where the segments themselves are stored. The AFF4 Image object is not
itself stored in any single volume.

| Predicate |  Description |
|-----------|--------------|
| *http://www.w3.org/1999/02/22-rdf-syntax-ns#type* | The type of object. For AFF4 Image objects this will be the URN *http://aff4.org/Schema#image* . |
| *http://aff4.org/Schema#chunk_size* | The (uncompressed) chunk size in bytes. This defaults to 32kb. Note that the last chunk may be shorter. |
| *http://aff4.org/Schema#chunks_per_segment* | How many chunks should be collected into a single bevy. Note that the last bevy may contain fewer chunks. |
| *http://aff4.org/Schema#compression* | How the chunks are compressed. Currently supported are *https://www.ietf.org/rfc/rfc1950.txt* (i.e. zlib.compress). |


### Example

In the following example we acquire a disk image into a new volume:

```sh
$ aff4imager -i /dev/sda1 -o /tmp/test.aff4 -t
Adding /dev/sda1
$ aff4imager -V /tmp/test.aff4
@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .
@prefix aff4: <http://aff4.org/Schema#> .
@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .

<aff4://05d7e827-5a23-4c17-97d5-190542e05b92/dev/sda1>
    aff4:chunk_size 32768 ;
    aff4:chunks_per_segment 1024 ;
    aff4:compression <https://www.ietf.org/rfc/rfc1950.txt> ;
    aff4:size 770786816 ;
    aff4:stored <aff4://05d7e827-5a23-4c17-97d5-190542e05b92> ;
    a aff4:image .

$ unzip -l /tmp/test.aff4
Archive:  /tmp/test.aff4
aff4://05d7e827-5a23-4c17-97d5-190542e05b92
  Length      Date    Time    Name
---------  ---------- -----   ----
      439  2015-02-24 17:06   information.turtle
 13859347  2015-02-24 17:06   dev/sda1/00000001
     4096  2015-02-24 17:06   dev/sda1/00000007/index
     4096  2015-02-24 17:06   dev/sda1/00000010/index
     4096  2015-02-24 17:06   dev/sda1/00000000/index
     4096  2015-02-24 17:06   dev/sda1/00000009/index
     3980  2015-02-24 17:06   dev/sda1/00000022/index
  7191448  2015-02-24 17:06   dev/sda1/00000015
     4096  2015-02-24 17:06   dev/sda1/00000017/index
     4096  2015-02-24 17:06   dev/sda1/00000008/index
     4096  2015-02-24 17:06   dev/sda1/00000019/index
....
```
