/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
#include <linux/string.h>
#include <linux/types.h>
#else
#include <string.h>
#include <stdlib.h>
#endif

#include "aesd-circular-buffer.h"


/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer. 
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
			size_t char_offset, size_t *entry_offset_byte_rtn )
{
	uint8_t index = 0;
	uint8_t max = (uint8_t) AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
	struct aesd_buffer_entry *entryptr = NULL;

	if(buffer->out_offs < buffer->in_offs)
	{
		for(index=buffer->out_offs, entryptr=&((buffer)->entry[index]); index<buffer->in_offs; index++, entryptr=&((buffer)->entry[index]))
		{
			if((int)(char_offset - ((entryptr->size) - 1)) <= 0)
			{
				*entry_offset_byte_rtn = char_offset;
				return entryptr;
			}
			else
			{
				char_offset = char_offset - (entryptr->size);
			}
		}
		return (struct aesd_buffer_entry *)NULL;
	}
	else
	{
		for(index=buffer->out_offs, entryptr=&((buffer)->entry[index]); index<max; index++, entryptr=&((buffer)->entry[index]))
		{
			if((int)(char_offset - ((entryptr->size) - 1)) <= 0)
			{
				*entry_offset_byte_rtn = char_offset;
				return entryptr;
			}
			else
			{
				char_offset = char_offset - (entryptr->size);
			}
		}
		for(index=0, entryptr=&((buffer)->entry[index]); index<buffer->in_offs; index++, entryptr=&((buffer)->entry[index]))
		{
			if((int)(char_offset - ((entryptr->size) - 1)) <= 0)
			{
				*entry_offset_byte_rtn = char_offset;
				return entryptr;
			}
			else
			{
				char_offset = char_offset - (entryptr->size);
			}
		}
		return (struct aesd_buffer_entry *)NULL;
	}
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
const char* aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
    const char *rbuf=NULL;
    uint8_t inoffset = buffer->in_offs;       
    
    if((buffer->in_offs == buffer->out_offs) && (buffer->full == true))
    {
    	rbuf = buffer->entry[buffer->out_offs].buffptr;
        buffer->out_offs = ((buffer->out_offs)+1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
    }

    buffer->entry[inoffset].buffptr = add_entry->buffptr;
    buffer->entry[inoffset].size = add_entry->size;
    buffer->in_offs = ((buffer->in_offs)+1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;

    if(buffer->in_offs==buffer->out_offs)
    {
        buffer->full=true;
    }

    return rbuf;
}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}
