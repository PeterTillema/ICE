.assume adl = 1
segment data
.def _FileiocheaderData

_FileiocheaderData:
	.db	0C0h, "FILEIOC", 0, 4
