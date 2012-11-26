#pragma once
#ifndef SCARC_GUARD_SOLVER_FUNCTOR_HH
#define SCARC_GUARD_SOLVER_FUNCTOR_HH 1

#include<kernel/base_header.hpp>
#include<kernel/scarc/scarc_error.hpp>
#include<kernel/foundation/functor.hpp>
#include<kernel/lafem/defect.hpp>
#include<kernel/lafem/sum.hpp>
#include<kernel/lafem/product.hpp>

using namespace FEAST::Foundation;
using namespace FEAST;

namespace FEAST
{
  namespace ScaRC
  {
    template<typename VT_>
    class SolverFunctorBase
    {
      public:
        ///needed in substitution of CSF
        typedef VT_ vector_type_;

        virtual void substitute(VT_& arg) = 0;

        virtual void execute() = 0;
        virtual const std::string type_name() = 0;

        virtual ~SolverFunctorBase()
        {
        }

        virtual bool is_complete()
        {
          return _complete;
        }

      protected:
        bool _complete;
    };


    ///always substitute left argument
    template<typename Algo_, typename VT_>
    class SumFunctorProxyLeft : public SolverFunctorBase<VT_>
    {
      public:
        SumFunctorProxyLeft(VT_& y, VT_& l, const VT_& r) :
          _y(y),
          _l(l),
          _r(r)
        {
          this->_complete = false;
        }

        virtual const std::string type_name()
        {
          return "SumFunctor";
        }

        virtual void execute()
        {
          if(!this->_complete)
            throw ScaRCError("Error: Incomplete SumFunctor can not be executed!");

          LAFEM::Sum<Algo_>::value(_y, _l, _r);
        }

        SumFunctorProxyLeft& operator=(const SumFunctorProxyLeft& rhs)
        {
          if(this == &rhs)
            return *this;

          this->_y = rhs._y;
          this->_l = rhs._l;
          this->_r = rhs._r;
          return *this;
        }

        SumFunctorProxyLeft(const SumFunctorProxyLeft& other) :
          _y(other._y),
          _l(other._l),
          _r(other._r)
        {
        }

        virtual void substitute(VT_& arg)
        {
          _l = arg;
          this->_complete = true;
        }

      private:
        VT_& _y;
        VT_& _l;
        const VT_& _r;
    };


    ///always substitute result and left arguments
    template<typename Algo_, typename VT_>
    class SumFunctorProxyResultLeft : public SolverFunctorBase<VT_>
    {
      public:
        SumFunctorProxyResultLeft(VT_& y, VT_& l, const VT_& r) :
          _y(y),
          _l(l),
          _r(r)
        {
          this->_complete = false;
        }

        virtual const std::string type_name()
        {
          return "SumFunctor";
        }

        virtual void execute()
        {
          if(!this->_complete)
            throw ScaRCError("Error: Incomplete SumFunctor can not be executed!");

          LAFEM::Sum<Algo_>::value(_y, _l, _r);
        }

        SumFunctorProxyResultLeft& operator=(const SumFunctorProxyResultLeft& rhs)
        {
          if(this == &rhs)
            return *this;

          this->_y = rhs._y;
          this->_l = rhs._l;
          this->_r = rhs._r;
          return *this;
        }

        SumFunctorProxyResultLeft(const SumFunctorProxyResultLeft& other) :
          _y(other._y),
          _l(other._l),
          _r(other._r)
        {
        }

        virtual void substitute(VT_& arg)
        {
          _y = arg;
          _l = arg;
          this->_complete = true;
        }

      private:
        VT_& _y;
        VT_& _l;
        const VT_& _r;
    };

    ///always substitute all arguments
    template<typename Algo_, typename VT_>
    class SumFunctorProxyAll : public SolverFunctorBase<VT_>
    {
      public:
        SumFunctorProxyAll(VT_& y, VT_& l, VT_& r) :
          _y(y),
          _l(l),
          _r(r)
        {
          this->_complete = false;
        }

        virtual const std::string type_name()
        {
          return "SumFunctor";
        }

        virtual void execute()
        {
          if(!this->_complete)
            throw ScaRCError("Error: Incomplete SumFunctor can not be executed!");

          LAFEM::Sum<Algo_>::value(_y, _l, _r);
        }

        SumFunctorProxyAll& operator=(const SumFunctorProxyAll& rhs)
        {
          if(this == &rhs)
            return *this;

          this->_y = rhs._y;
          this->_l = rhs._l;
          this->_r = rhs._r;
          return *this;
        }

        SumFunctorProxyAll(const SumFunctorProxyAll& other) :
          _y(other._y),
          _l(other._l),
          _r(other._r)
        {
        }

        virtual void substitute(VT_& arg)
        {
          _y = arg;
          _l = arg;
          _r = arg;
          this->_complete = true;
        }

      private:
        VT_& _y;
        VT_& _l;
        VT_& _r;
    };


    template<typename Algo_, typename VT_>
    class SumFunctor : public SolverFunctorBase<VT_>
    {
      public:
        SumFunctor(VT_& y, const VT_& l, const VT_& r) :
          _y(y),
          _l(l),
          _r(r)
        {
          this->_complete = true;
        }

        virtual const std::string type_name()
        {
          return "SumFunctor";
        }

        virtual void execute()
        {
          LAFEM::Sum<Algo_>::value(_y, _l, _r);
        }

        SumFunctor& operator=(const SumFunctor& rhs)
        {
          if(this == &rhs)
            return *this;

          this->_y = rhs._y;
          this->_l = rhs._l;
          this->_r = rhs._r;
          return *this;
        }

        SumFunctor(const SumFunctor& other) :
          _y(other._y),
          _l(other._l),
          _r(other._r)
        {
        }

        virtual void substitute(VT_& arg)
        {
        }

      private:
        VT_& _y;
        const VT_& _l;
        const VT_& _r;
    };

    ///always substitute right argument
    template<typename Algo_, typename VT_, typename MT_>
    class ProductFunctorProxyRight : public SolverFunctorBase<VT_>
    {
      public:
        ProductFunctorProxyRight(VT_& y, const MT_& l, VT_& r) :
          _y(y),
          _l(l),
          _r(r)
        {
          this->_complete = false;
        }

        virtual const std::string type_name()
        {
          return "ProductFunctor";
        }

        virtual void execute()
        {
          if(!this->_complete)
            throw ScaRCError("Error: Incomplete ProductFunctor can not be executed!");

          LAFEM::Product<Algo_>::value(_y, _l, _r);
        }

        ProductFunctorProxyRight& operator=(const ProductFunctorProxyRight& rhs)
        {
          if(this == &rhs)
            return *this;

          this->_y = rhs._y;
          this->_l = rhs._l;
          this->_r = rhs._r;
          return *this;
        }

        ProductFunctorProxyRight(const ProductFunctorProxyRight& other) :
          _y(other._y),
          _l(other._l),
          _r(other._r)
        {
        }

        virtual void substitute(VT_& arg)
        {
          _r = arg;
          this->_complete = true;
        }

      private:
        VT_& _y;
        const MT_& _l;
        VT_& _r;
    };

    ///always substitute result and right arguments
    template<typename Algo_, typename VT_, typename MT_>
    class ProductFunctorProxyResultRight : public SolverFunctorBase<VT_>
    {
      public:
        ProductFunctorProxyResultRight(VT_& y, const MT_& l, VT_& r) :
          _y(y),
          _l(l),
          _r(r)
        {
          this->_complete = false;
        }

        virtual const std::string type_name()
        {
          return "ProductFunctor";
        }

        virtual void execute()
        {
          if(!this->_complete)
            throw ScaRCError("Error: Incomplete ProductFunctor can not be executed!");

          LAFEM::Product<Algo_>::value(_y, _l, _r);
        }

        ProductFunctorProxyResultRight& operator=(const ProductFunctorProxyResultRight& rhs)
        {
          if(this == &rhs)
            return *this;

          this->_y = rhs._y;
          this->_l = rhs._l;
          this->_r = rhs._r;
          return *this;
        }

        ProductFunctorProxyResultRight(const ProductFunctorProxyResultRight& other) :
          _y(other._y),
          _l(other._l),
          _r(other._r)
        {
        }

        virtual void substitute(VT_& arg)
        {
          _y = arg;
          _r = arg;
          this->_complete = true;
        }

      private:
        VT_& _y;
        const MT_& _l;
        VT_& _r;
    };

    template<typename Algo_, typename VT_, typename MT_>
    class ProductFunctor : public SolverFunctorBase<VT_>
    {
      public:
        ProductFunctor(VT_& y, const MT_& l, const VT_& r) :
          _y(y),
          _l(l),
          _r(r)
        {
          this->_complete = true;
        }

        virtual const std::string type_name()
        {
          return "ProductFunctor";
        }

        virtual void execute()
        {
          LAFEM::Product<Algo_>::value(_y, _l, _r);
        }

        ProductFunctor& operator=(const ProductFunctor& rhs)
        {
          if(this == &rhs)
            return *this;

          this->_y = rhs._y;
          this->_l = rhs._l;
          this->_r = rhs._r;
          return *this;
        }

        ProductFunctor(const ProductFunctor& other) :
          _y(other._y),
          _l(other._l),
          _r(other._r)
        {
        }

        virtual void substitute(VT_& arg)
        {
        }

      private:
        VT_& _y;
        const MT_& _l;
        const VT_& _r;
    };

    template<typename Algo_, typename VT_>
    class PreconFunctor : public SolverFunctorBase<VT_>
    {
      public:
        PreconFunctor(VT_& y) :
          _y(y),
          _functor()
        {
          this->_complete = false;
        }

        virtual const std::string type_name()
        {
          return "PreconFunctor";
        }

        virtual void execute()
        {
          if(!this->_complete)
            throw ScaRCError("Error: Incomplete PreconFunctor can not be executed!");

          _functor->execute();
        }

        PreconFunctor& operator=(const PreconFunctor& rhs)
        {
          if(this == &rhs)
            return *this;

          this->_y = rhs._y;
          this->_functor = rhs._functor;
          return *this;
        }

        PreconFunctor(const PreconFunctor& other) :
          _y(other._y),
          _functor(other._functor)
        {
        }

        virtual void substitute(VT_& arg)
        {
        }

        void set_precon_functor(std::shared_ptr<SolverFunctorBase<VT_> >& functor)
        {
          functor->substitute(_y);
          _functor = functor;
          this->_complete = true;
        }

      private:
        VT_& _y;
        std::shared_ptr<SolverFunctorBase<VT_> > _functor;
    };

    template<typename Algo_, typename VT_>
    class CopyFunctor : public SolverFunctorBase<VT_>
    {
      public:
        CopyFunctor(VT_& y, const VT_& x) :
          _y(y),
          _x(x)
        {
          this->_complete = true;
        }

        virtual const std::string type_name()
        {
          return "CopyFunctor";
        }

        virtual void execute()
        {
          copy(_y, _x);
        }

        CopyFunctor& operator=(const CopyFunctor& rhs)
        {
          if(this == &rhs)
            return *this;

          this->_y = rhs._y;
          this->_x = rhs._x;
          return *this;
        }

        CopyFunctor(const CopyFunctor& other) :
          _y(other._y),
          _x(other._x)
        {
        }

        virtual void substitute(VT_& arg)
        {
        }

      private:
        VT_& _y;
        const VT_& _x;
    };

    template<typename Algo_, typename VT_, template<typename, typename> class StorageType_ = std::vector>
    class CompoundSolverFunctor : public SolverFunctorBase<VT_>
    {
      public:
        typedef StorageType_<std::shared_ptr<SolverFunctorBase<VT_> >, std::allocator<std::shared_ptr<SolverFunctorBase<VT_> > > > storage_type_;

        CompoundSolverFunctor() :
          _functors()
        {
          this->_complete = false;
        }

        virtual void substitute(VT_& arg)
        {
          for(Index i(0) ; i < (this->_functors).size() ; ++i)
          {
            (this->_functors).at(i)->substitute(arg);
          }
        }

        virtual void execute()
        {
          for(Index i(0) ; i < (this->_functors).size() ; ++i)
          {
            (this->_functors).at(i)->execute();
          }
        }

        virtual const std::string type_name()
        {
          return "CompoundSolverFunctor";
        }

        void add_functor(SolverFunctorBase<VT_>* functor)
        {
          _functors.push_back(std::shared_ptr<SolverFunctorBase<VT_> >(functor));
        }

        void add_functor(const std::shared_ptr<SolverFunctorBase<VT_> >& functor)
        {
          _functors.push_back(functor);
        }

        storage_type_& get_functors()
        {
          return _functors;
        }

        Index size()
        {
          return Index(_functors.size());
        }

        const storage_type_& get_functors() const
        {
          return _functors;
        }

        Index size() const
        {
          return Index(_functors.size());
        }

        CompoundSolverFunctor& operator=(const CompoundSolverFunctor& rhs)
        {
          if(this == &rhs)
            return *this;

          this->_functors = rhs._functors;
          return *this;
        }

        CompoundSolverFunctor(const CompoundSolverFunctor& other) :
          _functors(other._functors)
        {
        }

        template<typename OVT_>
        void set_preconditioner(SolverFunctorBase<OVT_>* precon_func)
        {
        }

        template<typename OVT_>
        void set_preconditioner(std::shared_ptr<SolverFunctorBase<OVT_> >& functor)
        {
          for(Index i(0) ; i < (this->_functors).size() ; ++i)
          {
            if( (this->_functors).at(i)->type_name() == "PreconFunctor")
            {
              ((PreconFunctor<Algo_, VT_>*)((this->_functors).at(i).get()))->set_precon_functor(functor);
              return;
            }
          }
          throw ScaRCError("Error: No PreconFunctor in functor list!");
        }

      protected:
        storage_type_ _functors;

    };

/*
    ///ApplicableAsPrecon interface
    template<typename MyFunctorType_, typename VT_>
    class ApplicableAsPrecon
    {
      public:
        ///needed in substitution of CSF
        typedef VT_ vector_type_;

        ///named copy-CTOR
        virtual MyFunctorType_ substitute(VT_& arg) = 0;
    };


    template<typename VT_>
    class ProxyPreconApplyFunctor : public FunctorBase
    {
      public:
        ProxyPreconApplyFunctor(VT_& x) :
          _x(x)
        {
        }

        virtual const std::string type_name()
        {
          return "ProxyPreconApplyFunctor";
        }

        virtual void execute()
        {
          throw ScaRCError("Error: Proxy functors can not be executed - substitute first!");
        }

        virtual void undo()
        {
          throw ScaRCError("Error: Numerical functors can not be undone!");
        }

        ProxyPreconApplyFunctor& operator=(const ProxyPreconApplyFunctor& rhs)
        {
          if(this == &rhs)
            return *this;

          this->_x = rhs._x;
          return *this;
        }

        ProxyPreconApplyFunctor(const ProxyPreconApplyFunctor& other) :
          _x(other._x)
        {
        }

        VT_& get_argument()
        {
          return _x;
        }

      private:
        ///apply preconditioner to what?
        VT_& _x;
    };

    template<template<typename, typename> class StorageType_ = std::vector>
    class CompoundSolverFunctor : public CompoundFunctor<StorageType_>
    {
      public:
        ///overwrite execute and undo functions
        virtual void execute()
        {
          for(Index i(0) ; i < (this->_functors).size() ; ++i)
          {
            (this->_functors).at(i)->execute();
          }
        }

        virtual const std::string type_name()
        {
          return "CompoundSolverFunctor";
        }

        virtual void undo()
        {
          throw ScaRCError("Error: Numerical functor lists can not be undone!");
        }

        ///substitute first ProxyPreconApplyFunctor by new_precon
        template<typename T_>
        void substitute_first(T_& new_precon)
        {
          if(new_precon.type_name() == "CompoundSolverFunctor")
          {
            for(Index i(0) ; i < (this->_functors).size() ; ++i)
            {
              if( (this->_functors).at(i)->type_name() == "ProxyPreconApplyFunctor")
              {
                ///TODO
              }
            }
          }
          else
          {
            for(Index i(0) ; i < (this->_functors).size() ; ++i)
            {
              if( (this->_functors).at(i)->type_name() == "ProxyPreconApplyFunctor")
              {
                (this->_functors).at(i) = std::shared_ptr<FunctorBase>(new T_(new_precon.substitute(((ProxyPreconApplyFunctor<typename T_::vector_type_>*)(this->_functors.at(i).get()))->get_argument())));
                return;
              }
            }
          }
          throw ScaRCError("Error: No ProxyPreconApplyFunctor in functor list!");
        }
    };

    template<typename Algo_, typename VT_, typename MT_>
    class DefectFunctor : public FunctorBase
    {
      public:
        DefectFunctor(VT_& d, const VT_& b, const MT_& A, const VT_& x) :
          _d(d),
          _b(b),
          _A(A),
          _x(x)
        {
        }

        virtual const std::string type_name()
        {
          return "DefectFunctor";
        }

        virtual void execute()
        {
          LAFEM::Defect<Algo_>::value(_d, _b, _A, _x);
        }

        virtual void undo()
        {
          throw ScaRCError("Error: Numerical functors can not be undone!");
        }

        DefectFunctor& operator=(const DefectFunctor& rhs)
        {
          if(this == &rhs)
            return *this;

          this->_d = rhs._d;
          this->_b = rhs._b;
          this->_A = rhs._A;
          this->_x = rhs._x;
          return *this;
        }

        DefectFunctor(const DefectFunctor& other) :
          _d(other._d),
          _b(other._b),
          _A(other._A),
          _x(other._x)
        {
        }

      private:
        VT_& _d;
        const VT_& _b;
        const MT_& _A;
        const VT_& _x;
    };


    template<typename Algo_, typename VT_>
    class SumFunctor : public FunctorBase
    {
      public:
        SumFunctor(VT_& y, const VT_& r, const VT_& l) :
          _y(y),
          _r(r),
          _l(l)
        {
        }

        virtual const std::string type_name()
        {
          return "SumFunctor";
        }

        virtual void execute()
        {
          LAFEM::Sum<Algo_>::value(_y, _r, _l);
        }

        virtual void undo()
        {
          throw ScaRCError("Error: Numerical functors can not be undone!");
        }

        SumFunctor& operator=(const SumFunctor& rhs)
        {
          if(this == &rhs)
            return *this;

          this->_y = rhs._y;
          this->_r = rhs._r;
          this->_l = rhs._l;
          return *this;
        }

        SumFunctor(const SumFunctor& other) :
          _y(other._y),
          _r(other._r),
          _l(other._l)
        {
        }

      private:
        VT_& _y;
        const VT_& _r;
        const VT_& _l;
    };


    template<typename Algo_, typename VT_, typename MT_>
    class ProductFunctor : public FunctorBase, public ApplicableAsPrecon<ProductFunctor<Algo_, VT_, MT_>, VT_>
    {
      public:
        ProductFunctor(VT_& y, const MT_& A, const VT_& x) :
          _y(y),
          _A(A),
          _x(x)
        {
        }

        virtual const std::string type_name()
        {
          return "ProductFunctor";
        }

        virtual void execute()
        {
          LAFEM::Product<Algo_>::value(_y, _A, _x);
        }

        virtual void undo()
        {
          throw ScaRCError("Error: Numerical functors can not be undone!");
        }

        ProductFunctor& operator=(const ProductFunctor& rhs)
        {
          if(this == &rhs)
            return *this;

          this->_y = rhs._y;
          this->_A = rhs._A;
          this->_x = rhs._x;
          return *this;
        }

        ProductFunctor(const ProductFunctor& other) :
          _y(other._y),
          _A(other._A),
          _x(other._x)
        {
        }

        ///implementation of ApplicableAsPrecon interface
        virtual ProductFunctor<Algo_, VT_, MT_> substitute(VT_& arg)
        {
          return ProductFunctor<Algo_, VT_, MT_>(arg, _A, arg);
        }

      private:
        VT_& _y;
        const MT_& _A;
        const VT_& _x;
    };
    */
  }
}

#endif
