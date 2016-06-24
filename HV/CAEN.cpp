// this file is distributed under 
// MIT license
#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <connectionstring.h>
#include <libhv_global.h>
#include <math_h/error.h>
#include "CAEN.h"
using namespace std;
using namespace HVAdjust;
using namespace MathTemplates;
const int ms_to_wait=300;
namespace Hardware{
	CAEN::CAEN(const string connstr){
		char buf[connstr.length()];
		for(size_t i=0; i<connstr.length();i++)buf[i]=connstr[i];
		LIBHV_createLibhvWithConnectionString(f_handle,buf,connstr.length());
		{
			int boardsCount=0;
			int* channelsCounts=nullptr;
			f_handle->getBordCountAndChannel(channelsCounts,boardsCount);
			channelsCounts=new int[boardsCount];
			f_handle->getBordCountAndChannel(channelsCounts,boardsCount);
			f_count=0;
			for(int i=0;i<boardsCount;i++)
				f_count+=channelsCounts[i];
			delete channelsCounts;
		}
		f_status_cache=new ChanelStatus*[f_count];
		for(size_t i=0;i<f_count;i++)
			f_status_cache[i]=new ChanelStatus();
		f_idx_cache=new size_t[f_count];
		UpdateRequest();
	}
	CAEN::~CAEN(){
		for(size_t i=0;i<f_count;i++)
			delete f_status_cache[i];
		delete[] f_status_cache;
		delete[] f_idx_cache;
		delete f_handle;
	}
	void CAEN::UpdateRequest(){
		this_thread::sleep_for(chrono::milliseconds(ms_to_wait));
		f_handle->getStatusForAll(f_status_cache,f_count);
		for(size_t i=0;i<f_count;i++){
			string idx_msg(f_status_cache[i]->getChannelName());
			idx_msg=idx_msg.substr(2,4);
			stringstream str(idx_msg);
			str>>f_idx_cache[i];
		}
	}
	const size_t CAEN::ChannelCount() const{return f_count;}
	ChanelStatus* CAEN::operator[](const size_t index)const{
		if(index>=f_count)
			throw Exception<CAEN>("Range check error");
		return f_status_cache[index];
	}
	const size_t CAEN::index2idx(const size_t index) const{
		if(index>=f_count)
			throw Exception<CAEN>("Range check error");
		return f_idx_cache[index];
	}
	const size_t CAEN::idx2index(const size_t idx) const{
		for(size_t i=0;i<f_count;i++)
			if(f_idx_cache[i]==idx)
				return i;
		throw Exception<CAEN>("IDX not found");
	}

	bool CAEN::IsOn(size_t idx) const{
		return string(f_status_cache[idx2index(idx)]->getStatus())=="ON";
	}
	double CAEN::GetHV(size_t idx) const{
		return f_status_cache[idx2index(idx)]->getVMon();
	}
	void CAEN::turnOn(size_t idx){
		this_thread::sleep_for(chrono::milliseconds(ms_to_wait));
		f_handle->switchChannel(idx,true);
	}
	void CAEN::turnOff(size_t idx){
		this_thread::sleep_for(chrono::milliseconds(ms_to_wait));
		f_handle->switchChannel(idx,false);
	}
	void CAEN::SetHV(size_t idx, double hv){
		this_thread::sleep_for(chrono::milliseconds(ms_to_wait));
		f_handle->setVoltage(idx,hv);
	}
}
