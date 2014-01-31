#pragma once

namespace Compiler
{
	class IVisitorBase
	{
	public:	
		virtual ~IVisitorBase() {};
	};

	template <typename T>
	class IVisitor : virtual public IVisitorBase
	{
	public:
		virtual void VisitOnEnter(T&) = 0;
		virtual void VisitOnLeave(T&) = 0;
	};

    class IVisitableBase
	{
	public:
		virtual ~IVisitableBase() {}
		virtual void AcceptOnEnter(IVisitorBase&) = 0;
		virtual void AcceptOnLeave(IVisitorBase&) = 0;

	protected:
		template <typename T>
		static void ConcreteAcceptOnEnter(T& visited, IVisitorBase& visitor)
		{
			if (IVisitor<T>* ptr = dynamic_cast<IVisitor<T>*>(&visitor))
			{
				return ptr->VisitOnEnter(visited);
			}
		}

		template <typename T>
		static void ConcreteAcceptOnLeave(T& visited, IVisitorBase& visitor)
		{
			if (IVisitor<T>* ptr = dynamic_cast<IVisitor<T>*>(&visitor))
			{
				return ptr->VisitOnLeave(visited);
			}
		}
	};

	// Inject this in visitable class
    #define COMPILER_DECLARE_VISITABLE()\
		public:\
			virtual void AcceptOnEnter(IVisitorBase& visitor)\
			{\
				return ConcreteAcceptOnEnter(*this, visitor);\
			}\
\
			virtual void AcceptOnLeave(IVisitorBase& visitor)\
			{\
				return ConcreteAcceptOnLeave(*this, visitor);\
			}\

}	//	namespace Compiler
