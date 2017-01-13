#include "meta.h"
#include "../util.h"

/* VGV (from Rune: Viking Warlord) */
VGMSTREAM * init_vgmstream_ps2_vgv(STREAMFILE *streamFile) {
    VGMSTREAM * vgmstream = NULL;
    char filename[1024];
    off_t start_offset;
    int loop_flag = 0;
	int channel_count;

    /* check extension, case insensitive */
    streamFile->get_name(streamFile,filename,sizeof(filename));
    if (strcasecmp("vgv",filename_extension(filename))) goto fail;

    /* check header */
    if (read_32bitBE(0x08,streamFile) != 0x0)
        goto fail;
    if (read_32bitBE(0x0C,streamFile) != 0x0)
        goto fail;

    loop_flag = 0;
    channel_count = 1;
    
	/* build the VGMSTREAM */
    vgmstream = allocate_vgmstream(channel_count,loop_flag);
    if (!vgmstream) goto fail;

	/* fill in the vital statistics */
    start_offset = 0x10;
	vgmstream->channels = channel_count;
    vgmstream->sample_rate = read_32bitLE(0x0,streamFile);
    vgmstream->coding_type = coding_PSX;
    vgmstream->num_samples = (get_streamfile_size(streamFile))*28/16/channel_count;
    if (loop_flag) {
        vgmstream->loop_start_sample = 0;
        vgmstream->loop_end_sample = (get_streamfile_size(streamFile)-start_offset)*28/16/channel_count;
    }

    vgmstream->layout_type = layout_none;
    vgmstream->meta_type = meta_PS2_VGV;

    /* open the file for reading */
    {
        int i;
        STREAMFILE * file;
        file = streamFile->open(streamFile,filename,STREAMFILE_DEFAULT_BUFFER_SIZE);
        if (!file) goto fail;
        for (i=0;i<channel_count;i++) {
            vgmstream->ch[i].streamfile = file;

            vgmstream->ch[i].channel_start_offset=
                vgmstream->ch[i].offset=start_offset; //+vgmstream->interleave_block_size*i;

        }
    }

    return vgmstream;

    /* clean up anything we may have opened */
fail:
    if (vgmstream) close_vgmstream(vgmstream);
    return NULL;
}
