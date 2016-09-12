ENTRY(main)

MEMORY
{
  STRT (rwx) : ORIGIN = 0, LENGTH = $STRT_LENGTH
  OVL (rwx) : ORIGIN = $STRT_LENGTH, LENGTH = $OVL_LENGTH
  APP (rwx) : ORIGIN = $STRT_LENGTH + $OVL_LENGTH, LENGTH = $APP_LENGTH
}

SECTIONS
{
	.header :
	{
		KEEP(*(.pbl_header))
	} > STRT

	/* -- DO NOT ADD ANY NEW SECTIONS HERE AND DO NOT CHANGE THE ALIGNMENT -- */
    /* The GNU build ID is tacked to the end of the PebbleProcessInfo struct: */
    .note.gnu.build-id ALIGN(1) : {
        PROVIDE(BUILD_ID = .);
        KEEP(*(.note.gnu.build-id))
    } > STRT

	OVERLAY : NOCROSSREFS AT ($STRT_LENGTH + $OVL_LENGTH + $APP_LENGTH)
	{
$OVL_CONTENT
	} > OVL

	.text : 
	{
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata*)
	} > APP

	.data :
	{
		KEEP(*(.data))
		*(.data.*)
		_ovly_table = .; 
$OVL_TABLE
		_novlys = .;
			LONG((_novlys - _ovly_table) / 12);
			
	} > APP

	.bss :
	{
		*(.bss)
		*(.bss.*)
	} > APP

	DISCARD :
	{
		libc.a ( * )
		libm.a ( * )
		libgcc.a ( * )
		*(.eh_frame)
	}
}