/**************************************************************************/
/*  camera_vita.cpp                                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "camera_vita.h"

CameraFeedVita::CameraFeedVita() {
	printf("Opencam\n");

	// name = "vita";
	// datatype = CameraFeed::FEED_RGB;
	// position = CameraFeed::FEED_UNSPECIFIED;

	img_data = malloc((320 * 240) * 3);

	cam_info.size = sizeof(SceCameraInfo);
	cam_info.format = SCE_CAMERA_FORMAT_ARGB;
	cam_info.resolution = SCE_CAMERA_RESOLUTION_320_240;
	cam_info.pitch = 0;
	cam_info.sizeIBase = (320 * 240);

	cam_info.pIBase = img_data;


	cam_info.framerate = SCE_CAMERA_FRAMERATE_120_FPS; //can be adjusted, especially at this resolution



	sceCameraOpen(SCE_CAMERA_DEVICE_BACK, &cam_info);

	printf("camstart\n");
	sceCameraStart(SCE_CAMERA_DEVICE_BACK);


	cam_thread.start(CameraFeedVita::update_image, this);
	// update_image();
};

void CameraFeedVita::update_image(void *p_udata) {
	CameraFeedVita *cf = (CameraFeedVita *)p_udata;
	cf->img_pool.resize((320 * 240) * 3);

	SceCameraRead cam_info_read;

	cam_info_read.size = sizeof(SceCameraRead);
	cam_info_read.mode = SCE_CAMERA_DEVICE_BACK;

	while(!cf->is_exit) {

		if(sceCameraIsActive(SCE_CAMERA_DEVICE_BACK))
			sceCameraRead(SCE_CAMERA_DEVICE_BACK, &cam_info_read);

		cf->lock();

		Ref<Image> image;

		PoolVector<uint8_t>::Write w = cf->img_pool.write();


		memcpy(w.ptr(), cf->img_data, (320 * 240) * 3);

		image.instance();

		image->create(320, 240, 0, Image::FORMAT_RGB8, cf->img_pool);

		image->convert(Image::FORMAT_RGB8);

		cf->set_RGB_img(image); //if this is called more than ~100 times = OOM
		cf->unlock();
		OS::get_singleton()->delay_usec(1000);
	}
	cf->is_exit = true;
}

CameraFeedVita::~CameraFeedVita() {
	if (is_active()) {
		deactivate_feed();
	};

	printf("Deinit\n");

	sceCameraStop(SCE_CAMERA_DEVICE_BACK);
	sceCameraClose(SCE_CAMERA_DEVICE_BACK);
	free(img_data);
};

bool CameraFeedVita::activate_feed() {
	is_exit = false;
	return true;
};

void CameraFeedVita::deactivate_feed() {
	is_exit = true;
};
void CameraFeedVita::lock() {
	mutex.lock();
};

void CameraFeedVita::unlock() {
	mutex.unlock();
};

void CameraVita::add_active_cameras() {
	Ref<CameraFeedVita> newfeed;
	newfeed.instance();
	add_feed(newfeed);
};

CameraVita::CameraVita() {
	add_active_cameras();
};

CameraVita::~CameraVita(){
	printf("destroy vita cam\n");
};
