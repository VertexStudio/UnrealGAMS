#include "GstAppSrcImpl.h"
#include "GstPipelineImpl.h"
#include "GstSampleImpl.h"

extern "C"
{
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
}

#include <vector>
#include <mutex>

class FGstAppSrcImpl : public IGstAppSrc
{
  public:
	FGstAppSrcImpl() {}
	~FGstAppSrcImpl() { Disconnect(); }
	virtual void Destroy();

	virtual bool Connect(IGstPipeline *Pipeline, const char *ElementName);
	virtual void Disconnect();
	virtual void PushTexture(const uint8_t *TextureData, size_t TextureSize);

  private:
	std::string m_Name;
	GstElement *m_AppSrc = nullptr;

	int m_Width = 0;
	int m_Height = 0;
};

IGstAppSrc *IGstAppSrc::CreateInstance()
{
	auto Obj = new FGstAppSrcImpl();
	GST_LOG_DBG_A("GstAppSrc: CreateInstance %p", Obj);
	return Obj;
}

void FGstAppSrcImpl::Destroy()
{
	GST_LOG_DBG_A("GstAppSrc: Destroy %p", this);
	delete this;
}

bool FGstAppSrcImpl::Connect(IGstPipeline *Pipeline, const char *ElementName)
{
	GST_LOG_DBG_A("GstAppSrc: Connect <%s>", ElementName);

	if (m_AppSrc)
	{
		GST_LOG_ERR_A("GstAppSrc: Already connected");
		return false;
	}

	for (;;)
	{
		m_Name = ElementName;

		m_AppSrc = gst_bin_get_by_name(GST_BIN(Pipeline->GetGPipeline()), ElementName);
		if (!m_AppSrc)
		{
			GST_LOG_ERR_A("gst_bin_get_by_name failed");
			break;
		}

		g_object_set(m_AppSrc, "emit-signals", TRUE, nullptr);

		GST_LOG_DBG_A("GstAppSrc: Connect SUCCESS");
		return true;
	}

	GST_LOG_ERR_A("GstAppSrc: Connect FAILED");
	Disconnect();
	return false;
}

void FGstAppSrcImpl::Disconnect()
{
	if (m_AppSrc)
	{
		GST_LOG_DBG_A("GstAppSrc: Disconnect <%s>", m_Name.c_str());
		gst_app_src_end_of_stream(GST_APP_SRC(m_AppSrc));
		g_object_set(m_AppSrc, "emit-signals", FALSE, nullptr);
		gst_object_unref(m_AppSrc);
		m_AppSrc = nullptr;
	}

	m_Width = 0;
	m_Height = 0;
}

void FGstAppSrcImpl::PushTexture(const uint8_t *TextureData, size_t TextureSize)
{
	GstBuffer *buffer = gst_buffer_new_allocate(nullptr, TextureSize, nullptr);
	gst_buffer_fill(buffer, 0, TextureData, TextureSize);
	const GstFlowReturn result = gst_app_src_push_buffer(GST_APP_SRC(m_AppSrc), buffer);
	GST_LOG_DBG_A("GstAppSrc: GstFlowReturn <%s> TextureSize <%i>", gst_flow_get_name(result), TextureSize);
}