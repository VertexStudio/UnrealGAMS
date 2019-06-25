#pragma once

#include "GstCoreImpl.h"

class IGstAppSrc
{
	GST_INTERFACE_IMPL(IGstAppSrc)

  public:
	virtual bool Connect(class IGstPipeline *Pipeline, const char *ElementName) = 0;
	virtual void Disconnect() = 0;
	virtual void PushTexture(const uint8_t *TextureData, size_t TextureSize) = 0;
};
