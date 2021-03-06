#ifndef CONTENT_COMMON_GPU_MEDIA_IMXVPU_VIDEO_DECODE_ACCELERATOR_H_
#define CONTENT_COMMON_GPU_MEDIA_IMXVPU_VIDEO_DECODE_ACCELERATOR_H_

#include <list>
#include <map>
#include <vector>

#include "base/compiler_specific.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/weak_ptr.h"

#include "base/message_loop/message_loop.h"
#include "base/compiler_specific.h"
#include "base/synchronization/lock.h"
#include "content/common/content_export.h"
#include "gpu/command_buffer/service/gles2_cmd_decoder.h"
#include "media/base/bitstream_buffer.h"
#include "media/video/picture.h"
#include "media/video/video_decode_accelerator.h"
//#include "media/gpu/gpu_video_decode_accelerator_factory_impl.h"
#include "media/gpu/gpu_video_decode_accelerator_helpers.h"

#include "imxvpucodec.h"
#include "imx_gl_viv_direct_texture.h"


namespace media
{


class CONTENT_EXPORT ImxVpuVideoDecodeAccelerator
	: public media::VideoDecodeAccelerator
{
public:
	explicit ImxVpuVideoDecodeAccelerator(base::WeakPtr < gpu::gles2::GLES2Decoder > const gles2_decoder, const MakeGLContextCurrentCallback& make_context_current_cb);
	virtual ~ImxVpuVideoDecodeAccelerator();

	virtual bool Initialize(const Config& config, Client *client) override;
	virtual void Decode(media::BitstreamBuffer const &bitstream_buffer) override;
	virtual void AssignPictureBuffers(std::vector < media::PictureBuffer > const &buffers) override;
	virtual void ReusePictureBuffer(int32_t picture_buffer_id) override;
	virtual void Flush() override;
	virtual void Reset() override;
	virtual void Destroy() override;
	virtual bool CanDecodeOnIOThread(); //override;

    static media::VideoDecodeAccelerator::SupportedProfiles GetSupportedProfiles();

private:
	enum ProcessRetval
	{
		ProcessOK,
		ProcessEOS,
		ProcessFailed
	};

	void Cleanup();

	// VPU specifics
	bool OpenDecoder();
	void CloseDecoder();
	bool AllocateVpuBitstreamBuffer();
	bool DeallocateVpuBitstreamBuffer();
	bool AllocateAndRegisterVPUFramebuffers();
	bool DeallocateVpuFramebuffers();

	// Bitstream buffer and framebuffer processing
	void ProcessQueuedInput();
	ProcessRetval ProcessInput(media::BitstreamBuffer const *input_bitstream_buffer);
	bool ProcessOutput(ImxVpuFramebuffer const &output_framebuffer, int32_t input_bitstream_buffer_id);

        bool TryToSetupDecodeOnSeparateThread(const base::WeakPtr<Client>& decode_client,
          const scoped_refptr<base::SingleThreadTaskRunner>& decode_task_runner);
     
	std::unique_ptr < base::WeakPtrFactory < Client > > client_ptr_factory_;
	base::WeakPtr < Client > client_;

	base::WeakPtr < gpu::gles2::GLES2Decoder > const gles2_decoder_;
	base::Callback < bool(void) > make_context_current_cb_;

	typedef std::vector < ImxVpuFramebuffer > ImxVpuFramebuffers;
	typedef std::vector < ImxVpuMemBlock > ImxVpuMemBlocks;
	ImxVpuDecoder *vpu_decoder_;
	ImxVpuCodecFormats codec_format_;
	ImxVpuMemBlock vpu_bitstream_buffer_block_;
	ImxVpuDecInitialInfo vpu_dec_initial_info_;
	ImxVpuFramebuffers vpu_framebuffers_;
	ImxVpuMemBlocks vpu_framebuffer_mem_blocks_;
	unsigned int aligned_width_, aligned_height_;
	bool initial_info_received_;

	GLESVIVDirectTextureProcs direct_texture_procs_;

	base::Lock lock_;

	typedef std::queue < media::BitstreamBuffer > BitstreamBufferQueue;
	BitstreamBufferQueue input_queue_;

	typedef std::map < int32_t, media::PictureBuffer > OutputBufferMap;
	OutputBufferMap output_picture_buffers_;

	// ChildThread's message loop
	base::MessageLoop* message_loop_;

	DISALLOW_COPY_AND_ASSIGN(ImxVpuVideoDecodeAccelerator);
};


} // namespace media end


#endif  // CONTENT_COMMON_GPU_MEDIA_IMXVPU_VIDEO_DECODE_ACCELERATOR_H_
