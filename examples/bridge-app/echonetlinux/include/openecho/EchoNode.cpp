/*
 * EchoNode.cpp
 *
 *  Created on: 2013/10/22
 *      Author: Fumiaki
 */

#include "EchoNode.h"
#include "OpenECHO.h"


namespace sonycsl_openecho {

EchoNode::EchoNode(std::shared_ptr<NodeProfile> nodeProfile,
		std::vector<std::shared_ptr<DeviceObject> > devices) {
	// self node
	mAddress = EchoSocket::SELF_ADDRESS;
	mNodeProfile = nodeProfile;
	int devicesSize = (int)devices.size();
	for(int i = 0; i < devicesSize; i++) {
		//devices.at(i).get()->allocateSelfDeviceInstanceCode();
		//mDevices.push_back(devices.at(i));
		//addDevice(devices.at(i));
		if(isSelfNode()) {
			devices.at(i).get()->allocateSelfDeviceInstanceCode();
			mDevices.push_back(devices.at(i));
		}
	}
}

EchoNode::EchoNode(std::string address) {
	// other node
	mAddress = address;
	mNodeProfile = std::shared_ptr<NodeProfile>(new NodeProfile::Proxy());
}


EchoNode::~EchoNode() {
	// TODO Auto-generated destructor stub
}

bool EchoNode::isProxy() {
	return (mAddress != EchoSocket::SELF_ADDRESS);
}

bool EchoNode::isSelfNode() {
	return (mAddress == EchoSocket::SELF_ADDRESS);
}

std::shared_ptr<NodeProfile> EchoNode::getNodeProfile() {
	return mNodeProfile;
}

std::string EchoNode::getAddress() {
	return mAddress;
}

std::shared_ptr<DeviceObject> EchoNode::addOtherDevice(unsigned short echoClassCode,
		unsigned char echoInstanceCode) {
	std::shared_ptr<DeviceObject> device(newOtherDevice(echoClassCode, echoInstanceCode));
	addDevice(device);
	return device;
}


void EchoNode::addDevice(std::shared_ptr<DeviceObject> device) {
	if(device.get() == nullptr) {
		return;
	}
	if(device.get()->getNode().get() == this) {
		return;
	}

	if(isSelfNode()) {
		device.get()->allocateSelfDeviceInstanceCode();
		device.get()->setNode(Echo::getSelfNode());
		mDevices.push_back(device);
		device.get()->onNew(device);
		device.get()->onFound(device);
	} else {
		mDevices.push_back(device);
	}

}
void EchoNode::removeDevice(unsigned short echoClassCode,
		unsigned char echoInstanceCode) {
	int listSize = (int)mDevices.size();
	for(int i = 0; i < listSize; i++) {
		if(mDevices.at(i).get()->getEchoClassCode() == echoClassCode
				&& mDevices.at(i).get()->getInstanceCode() == echoInstanceCode) {
			mDevices.erase(mDevices.begin() + i);

			if(isSelfNode()) {

			}
			break;
		}
	}
}

bool EchoNode::containsDevice(unsigned short echoClassCode,
		unsigned char echoInstanceCode) {

	int listSize = (int)mDevices.size();
	for(int i = 0; i < listSize; i++) {
		if(mDevices.at(i).get()->getEchoClassCode() == echoClassCode
				&& mDevices.at(i).get()->getInstanceCode() == echoInstanceCode) {

			return true;
		}
	}
	return false;
}

std::shared_ptr<EchoObject> EchoNode::getInstance(
		unsigned short echoClassCode, unsigned char echoInstanceCode) {
	if(echoClassCode == mNodeProfile.get()->getEchoClassCode()
			&& echoInstanceCode == mNodeProfile.get()->getInstanceCode()) {
		return mNodeProfile;
	}

	int listSize = (int)mDevices.size();
	for(int i = 0; i < listSize; i++) {
		if(mDevices.at(i).get()->getEchoClassCode() == echoClassCode
				&& mDevices.at(i).get()->getInstanceCode() == echoInstanceCode) {
			return mDevices.at(i);
		}
	}
	std::shared_ptr<EchoObject> eoj;
	return eoj;
}

std::vector<std::shared_ptr<DeviceObject> > EchoNode::getDevices() {

	return mDevices;
}


std::vector<std::shared_ptr<DeviceObject> > EchoNode::getDevices(
		unsigned short echoClassCode) {

	std::vector<std::shared_ptr<DeviceObject> > ret;

	int listSize = (int)mDevices.size();
	for(int i = 0; i < listSize; i++) {
		if(mDevices.at(i).get()->getEchoClassCode() == echoClassCode) {
			ret.push_back(mDevices.at(i));
		}
	}
	return ret;
}

void EchoNode::onNew(std::shared_ptr<EchoNode> node) {
	Echo::getEventListenerDelegate().onNewNode(node);
}

void EchoNode::onFound(std::shared_ptr<EchoNode> node) {
	Echo::getEventListenerDelegate().onFoundNode(node);
}

DeviceObject* EchoNode::newOtherDevice(unsigned short echoClassCode,
		unsigned char echoInstanceCode) {
	//return 0;
	DeviceObject* device = new DeviceObject::Proxy(echoClassCode, echoInstanceCode);

	return device;
}

};
