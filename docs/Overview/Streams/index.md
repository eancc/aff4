---
layout: docs
title: AFF4 Streams
order: 1
---

Streams are AFF4 objects which provide a readable interface to data. AFF4
streams present a common interface:

1) *Size()*: This method returns the size of the stream in bytes.
2) *Seek()*: This method allows randomly seeking in the stream.
3) *Read()*: This method reads some bytes from the stream at the current read pointer.
4) *Write()*: This method adds new data to the stream.
