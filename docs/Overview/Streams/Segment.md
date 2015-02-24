---
layout: docs
title: Segments
order: 2
---

AFF4 Segments are seekable units of data which are stored in a single unit. When
a segment is loaded, the entire segment exists in memory at the same time. For
this reason the segment size should be limited and not very large.

The *ZipFileSegment* is a special kind of segment which is stored in an AFF4
ZipFile volume. It is stored in a single archive member and *may* be compressed
inside the archive using the zlib deflate compression.