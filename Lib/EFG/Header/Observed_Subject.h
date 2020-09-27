#pragma once
#ifndef __EFG_OBSV_H__
#define __EFG_OBSV_H__

#include <list>
#include <stdlib.h>

namespace EFG {

	class Subject {
	public:
		virtual ~Subject() {};

		virtual bool is_observed() = 0;

		class Observer;	
	protected:
		Subject() {};
		
		virtual void add_Observer(Observer* obsv) = 0;
		virtual void rem_Observer(Observer* obsv) = 0;
	};

	class Subject::Observer {
	public:
		Observer(Subject* to_observe) : observed(to_observe) { this->observed->add_Observer(this); };
		~Observer(){ this->observed->rem_Observer(this); };
	private:
	// data
		Subject*           observed;
	};

	class Subject_singleObserver : public Subject {
	public:
		Subject_singleObserver() : __observer(nullptr) {};
		~Subject_singleObserver() { if(this->__observer != nullptr) abort(); };

		bool is_observed() override { return (this->__observer != nullptr); };
	private:
		void add_Observer(Observer* obsv) override { if(this->__observer != nullptr) abort(); this->__observer = obsv; };
		void rem_Observer(Observer* obsv) override { this->__observer = nullptr; };

		Observer*				__observer;
	};


	class Subject_multiObservers : public Subject {
	public:
		Subject_multiObservers() {};
		~Subject_multiObservers() { if(!this->__observers.empty()) abort(); };

		bool is_observed() override { return (!this->__observers.empty()); };
	private:
		void add_Observer(Observer* obsv) override { this->__observers.push_back(obsv); };
		void rem_Observer(Observer* obsv) override { this->__observers.remove(obsv); };

		std::list<Observer*>  	__observers;
	};

}

#endif