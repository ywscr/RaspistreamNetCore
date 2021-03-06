/********
Copyrights  Jean-Marie Boiry <jean-marie.boiry@live.fr>


This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/

#include "stdafx.h"
#include <iostream>
#include "Header.h"
#include "H264.h"
#include "CBinaryReader.h"
#include <WinSock2.h>
using namespace std;
#define _CRT_SECURE_NO_WARNINGS



#ifdef __cplusplus
extern "C" {
#endif
#define MMAL_PARAMETER_GROUP_CAMERA            (1<<16)

	MMAL_PORT_BH_CB_T _callback = NULL;
	char _buffer[17338402];
	MMAL_PORT_T *_port = NULL;

	__declspec(dllexport) void bcm_host_init()
	{

	}

	MMAL_PORT_T* CreatePort(const char* name)
	{
		MMAL_PORT_T* pPort = new MMAL_PORT_T();
		pPort->priv = new MMAL_PORT_PRIVATE_T();
		pPort->priv->buffer = NULL;
		pPort->is_enabled = 0;
		pPort->name = name;
		pPort->buffer_alignment_min = 1024;
		pPort->buffer_num = 1;
		pPort->format = new MMAL_ES_FORMAT_T();
		pPort->format->es = new MMAL_ES_SPECIFIC_FORMAT_T();

		return pPort;
	}

	__declspec(dllexport) MMAL_STATUS_T mmal_component_create(const char* name, MMAL_COMPONENT_T **component)
	{

		if (component == NULL)
			cout << "Null" << endl;


		MMAL_COMPONENT_T* p = new MMAL_COMPONENT_T();

		p->name = new char[100];
		strcpy_s((char*)p->name, 100, name);
		p->is_enabled = 0;

		p->control = CreatePort("Control");
		p->control->type = MMAL_PORT_TYPE_CONTROL;
		p->output_num = 3;
		p->output = new MMAL_PORT_T*[3];
		p->control->buffer_num = 3;
		p->control->buffer_num_recommended = 3;

		p->input_num = 3;
		p->input = new MMAL_PORT_T*[3];
		for (int i = 0; i < 3; i++)
		{
			char* szBuffer = new char[100];
			sprintf_s(szBuffer, 100, "%s: Output port %d", name, i);

			p->output[i] = CreatePort(szBuffer);
			p->output[i]->type = MMAL_PORT_TYPE_OUTPUT;
			p->output[i]->format->type = MMAL_ES_TYPE_VIDEO;
		}
		for (int i = 0; i < 3; i++)
		{
			char* szBuffer = new char[100];
			sprintf_s(szBuffer, 100, "%s: Input port %d", name, i);

			p->input[i] = CreatePort(szBuffer);
			p->input[i]->type = MMAL_PORT_TYPE_INPUT;
		}



		*component = p;
		return MMAL_SUCCESS;
	}

	__declspec(dllexport) MMAL_STATUS_T mmal_component_destroy(MMAL_COMPONENT_T *component)
	{
		delete component;
		return MMAL_SUCCESS;
	}

	__declspec(dllexport) MMAL_STATUS_T mmal_component_enable(MMAL_COMPONENT_T* component)
	{
		if (component->is_enabled)
			return MMAL_EINVAL;
		component->is_enabled = 1;
		return MMAL_SUCCESS;
	}

	__declspec(dllexport) MMAL_STATUS_T mmal_component_disable(MMAL_COMPONENT_T *component)
	{
		if (!component->is_enabled)
			return MMAL_EINVAL;
		component->is_enabled = 0;
		return MMAL_SUCCESS;
	}

	__declspec(dllexport) MMAL_STATUS_T mmal_connection_create(MMAL_CONNECTION_T **connection,
		MMAL_PORT_T *out, MMAL_PORT_T *in, uint32_t flags)
	{

		MMAL_CONNECTION_T* pConnection = new MMAL_CONNECTION_T();
		pConnection->is_enabled = 0;
		pConnection->in = in;
		pConnection->out = out;
		(*connection) = pConnection;
		return MMAL_SUCCESS;
	}

	__declspec(dllexport) MMAL_STATUS_T mmal_connection_destroy(MMAL_CONNECTION_T *connection)
	{
		delete connection;
		return MMAL_SUCCESS;
	}

	__declspec(dllexport) MMAL_STATUS_T mmal_connection_disable(MMAL_CONNECTION_T *connection)
	{
		connection->is_enabled = 0;
		return MMAL_SUCCESS;
	}

	__declspec(dllexport) MMAL_STATUS_T mmal_connection_enable(MMAL_CONNECTION_T *connection)
	{
		if (connection->is_enabled)
			return MMAL_EINVAL;

		connection->is_enabled = 1;
		return MMAL_SUCCESS;
	}

	MMAL_POOL_T* pool = NULL;

	__declspec(dllexport) MMAL_POOL_T *mmal_port_pool_create(MMAL_PORT_T *port, unsigned int headers, uint32_t payload_size)
	{
		pool = new MMAL_POOL_T();
		pool->queue = new MMAL_QUEUE_T;
		pool->queue->current = 0;
		pool->queue->buffers[0] = new MMAL_BUFFER_HEADER_T;
		memset(pool->queue->buffers[0], 0, sizeof(MMAL_BUFFER_HEADER_T));
		//pool->queue->buffers[1] = new MMAL_BUFFER_HEADER_T;
		pool->header = pool->queue->buffers;
		pool->headers_num = 1;
		//pool->header[0] = new MMAL_BUFFER_HEADER_T;

		return pool;
	}

	__declspec(dllexport) MMAL_BUFFER_HEADER_T *mmal_queue_get(MMAL_QUEUE_T *queue)
	{
		_ASSERT(pool->queue == queue);
		int current = pool->queue->current;
		pool->queue->current;
		/*if (pool->queue->current == 2)
			pool->queue->current = 0;
*/
		MMAL_BUFFER_HEADER_T* buffer = pool->queue->buffers[current];
		buffer->length = 0;// 4096;
		buffer->data = NULL;// new byte[4096];
		return buffer;
	}

	__declspec(dllexport) void mmal_pool_destroy(MMAL_POOL_T *pool1)
	{
		_ASSERT(pool == pool1);
		delete pool->queue->buffers[0];
		delete pool->queue;
		//delete pool->header;
		//delete[] pool->header;
		delete pool;
		pool = NULL;
	}

	

	__declspec(dllexport) unsigned int mmal_queue_length(MMAL_QUEUE_T *queue)
	{
		_ASSERT(pool->queue == queue);
		return 1;
	}
	bool bStop;

#ifdef DONE
	DWORD WINAPI ThreadProc(LPVOID lpParameter)
	{
		MMAL_PORT_T *port = (MMAL_PORT_T*)lpParameter;

		for (int i = 0; i <= 1831; i++)
		{

			char filePath[512];
			sprintf(filePath, "D:\\source\\repos\\NetCore\\RaspiCam\\Debug\\Frames\\Frame%d.h264", i);
			FILE* fp = fopen(filePath, "rb");
			fseek(fp, 0, SEEK_END);
			int _size = ftell(fp);
			byte* _buffer = new byte[_size];
			fseek(fp, 0, SEEK_SET);
			int read = fread(_buffer, _size, 1, fp);
			if (read != 1)
				throw "Invalid read";


			auto_ptr<CMemoryStream> stream(new CMemoryStream(_buffer, _size));
			auto_ptr<CBinaryReader> br(new CBinaryReader(stream.get()));

			unsigned flags = br->ReadInt32();
			__int64 timeStamp = br->ReadInt64();
			//printf("Frame %d, flags %d, timestamp %I64d len %d\n",i , flags, timeStamp, _size - 12);
			byte* buffer = new byte[_size - 12];
			stream->Read(buffer, 0, _size - 12);


			MMAL_BUFFER_HEADER_T* buff = new  MMAL_BUFFER_HEADER_T();
			buff->length = _size - 12;
			buff->data = (uint8_t*)buffer;
			buff->pts = timeStamp;
			buff->flags = flags;

			_callback(port, buff);
			//1831 => 60 secs
			// 1831 / 60 = 1 sec

			Sleep(0);

			fclose(fp);
			if (bStop)
				break;
		}
		MMAL_BUFFER_HEADER_T* buff = new  MMAL_BUFFER_HEADER_T();
		buff->length = 0;
		buff->data = NULL;
		buff->flags |= MMAL_BUFFER_HEADER_FLAG_EOS;
		printf("Thread exiting\n");
		return 0;
	}
#else
	HANDLE hEvent = NULL;
	DWORD WINAPI ThreadProc(LPVOID lpParameter)
	{
		MMAL_PORT_T *port = (MMAL_PORT_T*)lpParameter;
		_ASSERT(port->priv->buffer = pool->queue->buffers[0]);
		int nalNumber = 0;
		H264 theH264;
		TCHAR szPath[MAX_PATH];
		wchar_t drive[_MAX_DRIVE];
		wchar_t dir[_MAX_PATH];
		wchar_t fname[_MAX_FNAME];
		wchar_t ext[_MAX_EXT];

		GetModuleFileName(GetModuleHandle(NULL), szPath, MAX_PATH);

		_wsplitpath(szPath, drive, dir, fname, ext);
		/*{
			_ASSERT(false);
		}*/
		

		theH264.SetBuffer("D:\\AlohaBig.h264");
		_ASSERT(_callback);
		_port = port;
		int size = 0;
		int nal_end = 0;
		if (theH264.gf_media_nalu_is_start_code() == 0)
			throw "new Exception(No Start Code)";
		int nal_start = theH264.Position();
		int headerSize = theH264.Position();
		__int64 totalTime = 70000000;
		int timeIncrement = totalTime / 823;
		int base = 1000000;

		_ASSERT(theH264.Position() == 4);
		while (theH264.Position() < theH264.Length())
		{
			int nal_and_trailing_size;
			int nal_size;
			bool skip_nal = false;
			int add_sps;
			//if (iter == 3)
			//	Console.WriteLine("Break");
			// Here I'm after the start code

			nal_and_trailing_size = nal_size = theH264.gf_media_nalu_next_start_code_bs();

			if (true)
				nal_size = theH264.gf_media_nalu_payload_end_bs();

			//printf("nal start %d, nal_end %d nal_size %d stream pos %d\n", nal_start, nal_end, nal_size, theH264.Position());

			/*if (nal_start == 0)
				theH264._ms->Seek(nal_start);
			*/

			theH264._ms->Seek(theH264.Position() - headerSize);


			byte* buffer = new byte[nal_size + headerSize];
			theH264._ms->Read(buffer, 0, (int)nal_size + headerSize);
			theH264._ms->Seek(nal_start);



			MMAL_BUFFER_HEADER_T* buff = port->priv->buffer;
			buff->length = nal_size + headerSize;
			buff->data = (uint8_t*)buffer;
			buff->pts = base;

			buff->flags = MMAL_BUFFER_HEADER_FLAG_FRAME_END;
			base += timeIncrement;
			nalNumber++;
			_callback(_port, buff);
			if (bStop)
			{
				SetEvent(hEvent);
				goto done;

			}
			Sleep(15);
			delete[] buffer;
				
					   
			nal_end = (int)theH264.Position();// gf_bs_get_position(bs);
			_ASSERT(nal_start <= nal_end);
			_ASSERT(nal_end <= nal_start + nal_and_trailing_size + headerSize);
			if (nal_end != nal_start + nal_and_trailing_size)
				theH264._ms->Seek(nal_start + nal_and_trailing_size);

			if (nal_size == 0)
				break;

			if (theH264.Position() == theH264._ms->Length())
				break;
			//if (duration && (dts_inc * cur_samp > duration)) break;
			//if (import->flags & GF_IMPORT_DO_ABORT) break;

			/*consume next start code*/
			int pos = theH264.Position();
			nal_start = theH264.gf_media_nalu_next_start_code_bs();
			if (nal_start != 0)
			{
				printf("[avc-h264] invalid nal_size ({%d})? Skipping {%d} bytes to reach next start code\n", nal_size, nal_start);
				//gf_bs_skip_bytes(bs, nal_start);
			}
			nal_start = theH264.gf_media_nalu_is_start_code();
			if (nal_start == 0)
			{
				//GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[avc-h264] error: no start code found ("LLU" bytes read out of "LLU") - leaving\n", gf_bs_get_position(bs), gf_bs_get_size(bs)));
				break;
			}
			headerSize = theH264.Position() - pos;
			nal_start = theH264.Position();// 
		}
			
		
done:
		//printf("NalNumber for 70 sec : %d\n", nalNumber);
		/*MMAL_BUFFER_HEADER_T* buff = new  MMAL_BUFFER_HEADER_T();
		buff->length = 0;
		buff->data = new byte[1];
		buff->flags |= MMAL_BUFFER_HEADER_FLAG_EOS;

		_callback(port, buff);*/
		printf("Thread exiting\n");
		return 0;
	}
#endif

	__declspec(dllexport) MMAL_STATUS_T mmal_port_parameter_set(MMAL_PORT_T* port, const MMAL_PARAMETER_HEADER_T* param)
 {
	 if (param->id == MMAL_PARAMETER_CAMERA_CONFIG)
	 {
		 MMAL_PARAMETER_CAMERA_CONFIG_T* ptr = (MMAL_PARAMETER_CAMERA_CONFIG_T*)param;
	 }
	 if (param->id == MMAL_PARAMETER_PROFILE)
	 {
		MMAL_PARAMETER_VIDEO_PROFILE_T* par = (MMAL_PARAMETER_VIDEO_PROFILE_T*)param;
	 }

	 return MMAL_SUCCESS;
 }

 __declspec(dllexport) MMAL_STATUS_T  mmal_port_parameter_set_uint32(MMAL_PORT_T *port, uint32_t id, uint32_t value)
 {
	 return MMAL_SUCCESS;
}

#define FRAME
#ifdef FRAME
 HANDLE hnd;
 __declspec(dllexport) MMAL_STATUS_T mmal_port_parameter_set_boolean(MMAL_PORT_T *port, uint32_t id, MMAL_BOOL_T value)
 {

	 if (MMAL_PARAMETER_CAPTURE == id && value != 0)
	 {
		 hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		 port->is_enabled = 1;
		 /*FILE* fp = fopen(, "rb");
		 fseek(fp, 0, SEEK_END);
		 int size = ftell(fp);
		 fseek(fp, 0, SEEK_SET);
		 if (size > sizeof(_buffer))
			 cout << "Buffer to small" << endl;
		 int read = fread(_buffer, size, 1, fp);
		 if (read != 1)
			 throw "Invalid read";*/
		
		 bStop = false;
		 HANDLE hnd = CreateThread(NULL, 0, ThreadProc, port, 0, NULL);
		 CloseHandle(hnd);

		
		 //_callback(port, buff);
	 }
	 else if (MMAL_PARAMETER_CAPTURE == id && value == 0)
	 {
		 bStop = true;
		 WaitForSingleObject(hEvent, INFINITE);
		 printf("Thread killed\n");
		 CloseHandle(hEvent);
		 hEvent = NULL;
		 //SuspendThread(hnd);
	 }
	 return MMAL_SUCCESS;
 }

#else

 __declspec(dllexport) MMAL_STATUS_T mmal_port_parameter_set_boolean(MMAL_PORT_T *port, uint32_t id, MMAL_BOOL_T value)
 {
	 
	 if (MMAL_PARAMETER_CAPTURE == id && value != 0)
	 {
		 port->is_enabled = 1;
		 H264 theH264;
		 /*FILE* fp = fopen(, "rb");
		 fseek(fp, 0, SEEK_END);
		 int size = ftell(fp);
		 fseek(fp, 0, SEEK_SET);
		 if (size > sizeof(_buffer))
			 cout << "Buffer to small" << endl;
		 int read = fread(_buffer, size, 1, fp);
		 if (read != 1)
			 throw "Invalid read";*/
		 theH264.SetBuffer("D:\\source\\repos\\NetCore\\RaspiCam\\Debug\\Aloha.h264");
		 _ASSERT(_callback);
		 _port = port;
		 int size = 0;
		 int nal_end = 0;
		 if (theH264.gf_media_nalu_is_start_code() == 0)
			 throw "new Exception(No Start Code)";
		 int nal_start = theH264.Position();
		 int headerSize = theH264.Position();
		 int nalNumber = 0;
		 __int64 totalTime = 70000000;
		 int timeIncrement = totalTime / 823;
		 int base = 1000000;

		 _ASSERT(theH264.Position() == 4);
		 while (theH264.Position() < theH264.Length())
		 {
			 int nal_and_trailing_size;
			 int nal_size;
			 bool skip_nal = false;
			 int add_sps;
			 //if (iter == 3)
			 //	Console.WriteLine("Break");
			 // Here I'm after the start code
			
			 nal_and_trailing_size = nal_size = theH264.gf_media_nalu_next_start_code_bs();

			 if (true)
				 nal_size = theH264.gf_media_nalu_payload_end_bs();
			
			 //printf("nal start %d, nal_end %d nal_size %d stream pos %d\n", nal_start, nal_end, nal_size, theH264.Position());

			/*if (nal_start == 0)
				theH264._ms->Seek(nal_start);
			*/ 
			
			theH264._ms->Seek(theH264.Position() -  headerSize);
			

			 byte* buffer = new byte[nal_size + headerSize];
			 theH264._ms->Read(buffer, 0, (int)nal_size + headerSize);
			 theH264._ms->Seek(nal_start);



			 MMAL_BUFFER_HEADER_T* buff = new  MMAL_BUFFER_HEADER_T();
			 buff->length = nal_size + headerSize;
			 buff->data = (uint8_t*)buffer;
			 buff->pts = base;

			 buff->flags = MMAL_BUFFER_HEADER_FLAG_FRAME_END;
			 base += timeIncrement;
			 nalNumber++;
			_callback(_port, buff);
			
			//delete[] buffer;
			//delete buff;

			

			 nal_end = (int)theH264.Position();// gf_bs_get_position(bs);
			 _ASSERT(nal_start <= nal_end);
			 _ASSERT(nal_end <= nal_start + nal_and_trailing_size + headerSize);
			 if (nal_end != nal_start + nal_and_trailing_size)
				 theH264._ms->Seek(nal_start + nal_and_trailing_size);

			 if (nal_size == 0)
				 break;

			 if (theH264.Position() == theH264._ms->Length())
				 break;
			 //if (duration && (dts_inc * cur_samp > duration)) break;
			 //if (import->flags & GF_IMPORT_DO_ABORT) break;

			 /*consume next start code*/
			 int pos = theH264.Position();
			 nal_start = theH264.gf_media_nalu_next_start_code_bs();
			 if (nal_start != 0)
			 {
				 printf("[avc-h264] invalid nal_size ({%d})? Skipping {%d} bytes to reach next start code\n", nal_size, nal_start);
				 //gf_bs_skip_bytes(bs, nal_start);
			 }
			 nal_start = theH264.gf_media_nalu_is_start_code();
			 if (nal_start == 0)
			 {
				 //GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[avc-h264] error: no start code found ("LLU" bytes read out of "LLU") - leaving\n", gf_bs_get_position(bs), gf_bs_get_size(bs)));
				 break;
			 }
			 headerSize = theH264.Position() - pos;
			 nal_start = theH264.Position();// gf_bs_get_position(bs);
		 }
		 printf("NalNumber for 70 sec : %d\n", nalNumber);
		 MMAL_BUFFER_HEADER_T* buff = new  MMAL_BUFFER_HEADER_T();
		 buff->length = 0;
		 buff->data = NULL;
		 buff->flags |= MMAL_BUFFER_HEADER_FLAG_EOS;

		 _callback(port, buff);
	 }
	 return MMAL_SUCCESS;
 }
#endif
 __declspec(dllexport) MMAL_STATUS_T mmal_port_format_commit(MMAL_PORT_T* port)
 {
	 return MMAL_SUCCESS;
 }

 /* Get a port parameter */
 __declspec(dllexport) MMAL_STATUS_T mmal_port_parameter_get(MMAL_PORT_T *port,	 MMAL_PARAMETER_HEADER_T *param)
 {
	 MMAL_STATUS_T status = MMAL_ENOSYS;
	
	 
	 if (!param)
		 return MMAL_EINVAL;

	 MMAL_PARAMETER_CAMERA_INFO_T* info = (MMAL_PARAMETER_CAMERA_INFO_T*)param;
	 info->num_cameras = 1;
	 strcpy_s(info->cameras[0].camera_name, 16, "vc.cil.camera");
	 info->cameras[0].max_height = 100;
	 info->cameras[0].max_width = 200;


	 return status;
 }

 __declspec(dllexport) MMAL_STATUS_T mmal_port_enable(MMAL_PORT_T *port, MMAL_PORT_BH_CB_T cb)
 {
	 MMAL_BUFFER_HEADER_T* p = new MMAL_BUFFER_HEADER_T();

	 if (port->is_enabled)
		 return MMAL_EINVAL;

	 if (cb != NULL)
	 {
		 _callback = cb;
	 }

	 port->is_enabled = 1;

	// cb(port, p);
	 return MMAL_SUCCESS;
 }

 __declspec(dllexport) MMAL_STATUS_T mmal_port_disable(MMAL_PORT_T *port)
 {
	 if (!port->is_enabled)
		 return MMAL_EINVAL;

	 port->is_enabled = 0;

	 return MMAL_SUCCESS;
 }

 __declspec(dllexport) void mmal_format_copy(MMAL_ES_FORMAT_T *format_dest, MMAL_ES_FORMAT_T *format_src)
 {

 }

 __declspec(dllexport)void mmal_buffer_header_release(MMAL_BUFFER_HEADER_T *header)
 {
	 
 }

 __declspec(dllexport) MMAL_STATUS_T mmal_port_send_buffer(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
 {
	 _ASSERT(pool->queue->buffers[0] == buffer);
	 port->priv->buffer = buffer;
	 return MMAL_SUCCESS;
 }

 __declspec(dllexport)MMAL_STATUS_T mmal_buffer_header_mem_lock(MMAL_BUFFER_HEADER_T *header)
 {
	 
	 return MMAL_SUCCESS;
 }

 /** Unlock the data buffer contained in the buffer header.
  * This call does nothing on all platforms except VideoCore where it is needed to un-pin a
  * buffer in memory after any access to it.
  *
  * @param header buffer header to unlock
  */
 __declspec(dllexport)void mmal_buffer_header_mem_unlock(MMAL_BUFFER_HEADER_T *header)
 {
 }


#ifdef __cplusplus
}
#endif
