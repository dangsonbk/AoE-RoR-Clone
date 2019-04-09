These following pieces of document I got while searching in Google for SLP file format.

--- 
Source: [digitization.org](http://www.digitization.org/wiki/index.php/SLP) | [Wayback Machine](https://web.archive.org/web/20130411064914/http://www.digitization.org/wiki/index.php/SLP)

Genie uses the SLP format to store its graphics. The reason behind the name "SLP" is unknown, but it is known that Ensemble Studios used the name internally because each file has the string "ArtDesk 1.00 SLP Writer" embedded in it. The format allows for special features such as player colors, but is a bit dated when compared to the possibilities modern standards (such as PNG) allow. Following is a description of the format.

### Data format

The first item in an SLP file is the header, of the following format:

```c
struct file_header
{
	char  version[4];  // Given as a string, usually "2.0N".
	long  num_frames;  // The total number of frames in the SLP.
	char  comment[24]; // The above string "ArtDesk 1.00 SLP Writer".
};
```

Following is frame information, one of the following structures for each frame (see num_frames above):

```c
struct frame_info
{
	unsigned long	cmd_table_offset;
	unsigned long	outline_table_offset;
	unsigned long	palette_offset;
	unsigned long	properties;
	long	width;
	long	height;
	long	hotspot_x;
	long	hotspot_y;
};
```

properties: This value is suspected to change which palette the SLP uses, but the possible options seem to be the same so this has never been confirmed.
hotspot_x, hotspot_y: These points set the hotspot of a frame, which is the "center" of the unit. A unit's coordinates in a scenario specify the location of this pixel.

### Per-frame data

The following data are repeated as a whole for each frame.

The first data are edge structures, one for each row. To conserve disk space, commands (see below) are only included for the necessary area, and these edges specify the blank space surrounding a unit. Rows that do not have any data in them (i.e completely transparent) will have left and right values of 0x8000. There are no "End of Row" command bytes for these rows.

```c
struct rowedge
{
	short left, right;
};
```

Following are an array of longs, specifying the offset for the commands (see below) for each row. These are not actually necessary as the commands can be read sequentially, but can be used for checking purposes.

After the array is the command data itself. Each command is a single-byte integer followed by command-specific data. To make the data very compact, the command byte sometimes also contains the command-specific data. For example, the Color List command (0x00) only uses the least-significant bits to specify the command type and the rest of the byte is the number of palette indices to follow.

The following are the known commands:

```
Value	Command name	Pixel count	Description
0x00	Color list	>>2	An array of palette indices. This is about as bitmap as it gets in SLPs.
0x01	Skip	>>2 or next	The specified number of pixels are transparent.
0x02	Big color list	>>4 * 256 and next	An array of palette indexes. Supports a greater number of pixels than the above color list.
0x03	Big skip	>>4 * 256 and next	The specified number of pixels are transparent. Supports a greater number of pixels than the above skip.
0x06	Player color list	>>4 or next	An array of player color indexes. The actual palette index is given by adding ([player number] * 16) + 16 to these values.
0x07	Fill	>>4 or next	Fills the specified number of pixels with the following palette index.
0x0A	Player color fill	>>4 or next	Same as above, but using the player color formula (see Player color list).
0x0B	Shadow transparent	>>4 or next	Specifies the shadow for a unit, but most SLPs do not use this.
0x0E	Shadow player	next	Unknown.
0x0F	End of row	0	Specifies the end of commands for this row.
0x4E	Outline	1	When normally drawing a unit, this single pixel acts normally. When the unit is behind something, this pixel is replaced with the player color.
0x5E	Outline span	next	Same function as above, but supports multiple pixels.
```

### Possible pixel count values:

```
Next byte: Stored in the next byte.
>>2: Command byte right-shifted 2 bits.
>>4 or next: Either the command byte right-shifted 4 bits or in the next byte if the former is 0.
>>2 or next: Either the command byte right-shifted 2 bits or in the next byte if the former is 0.
>>2 * 256 and next: The command byte right-shifted 2 bits and then added to the next byte's value.
>>4 * 256 and next: The command byte right-shifted 4 bits and then added to the next byte's value.
```
---
Other links:
- [libaof](https://github.com/chrisforbes/libaof/blob/master/libaof/slp.c)
- [SFTtech](https://github.com/SFTtech/openage/blob/master/doc/media/slp-files.md)