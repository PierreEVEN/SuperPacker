#pragma once

// SOURCE : https://github.com/PierreEVEN/CppUtils/
// This is a delegate system I've created for my C++ projects
// Usage : declare a delegate type using the appropriate macro : DECLARE_DELEGATE_TYPE(EventMyDelegate), then declare a property using this type.

#include <cassert>
#include <memory>
#include <vector>

#define DECLARE_DELEGATE_SINGLECAST(name, ...)                    using name = DelegateSingleCast<void, __VA_ARGS__>;
#define DECLARE_DELEGATE_SINGLECAST_RETURN(name, returnType, ...) typedef DelegateSingleCast<returnType, __VA_ARGS__> name;
#define DECLARE_DELEGATE_MULTICAST(name, ...)                     using name = DelegateMultiCast<__VA_ARGS__>;

template <typename Return_T, typename... Args_T> class DelegateFunctionPtrWrapper
{
  public:
    virtual Return_T execute(Args_T&...)           = 0;
    virtual bool     operator==(const void*) const = 0;
};

template <typename... Args_T> class ILambdaClassStorage
{
  public:
    ILambdaClassStorage()                     = default;
    virtual void execute(Args_T&&... in_args) = 0;
};

template <typename Lambda_T, typename... Args_T> class TLambdaClassStorage final : public ILambdaClassStorage<Args_T...>
{
  public:
    TLambdaClassStorage(Lambda_T in_lambda) : lambda_expression(in_lambda)
    {
    }
    void execute(Args_T&&... in_args) override
    {
        lambda_expression(std::forward<Args_T>(in_args)...);
    }

  private:
    Lambda_T lambda_expression;
};

template <typename ObjectClass_T, typename Return_T, typename... Args_T> class DelegateFunctionPtr final : public DelegateFunctionPtrWrapper<Return_T, Args_T...>
{
  public:
    DelegateFunctionPtr(ObjectClass_T* objPtr, Return_T (ObjectClass_T::*funcPtr)(Args_T...)) : object_ptr(objPtr), function_ptr(funcPtr)
    {
    }

    Return_T execute(Args_T&... inArgs)
    {
        return (object_ptr->*function_ptr)(inArgs...);
    }

    bool operator==(const void* objPtr) const
    {
        return object_ptr == objPtr;
    }

  private:
    ObjectClass_T* object_ptr;
    Return_T (ObjectClass_T::*function_ptr)(Args_T...);
};

template <typename Return_T, typename... Args_T> class DelegateSingleCast final
{
  public:
    ~DelegateSingleCast()
    {
        clear();
    }

    template <typename ObjectClass_T> void bind(ObjectClass_T* inObjPtr, Return_T (ObjectClass_T::*inFunc)(Args_T...))
    {
        function_ptr = std::make_shared<DelegateFunctionPtr<ObjectClass_T, Return_T, Args_T...>>(inObjPtr, inFunc);
    }

    void clear()
    {
        function_ptr = nullptr;
    }

    Return_T execute(Args_T&... inArgs)
    {
        assert(function_ptr);
        return function_ptr->execute(inArgs...);
    }

  private:
    std::shared_ptr<DelegateFunctionPtrWrapper<Return_T, Args_T...>> function_ptr = nullptr;
};

template <typename... Args_T> class DelegateMultiCast final
{
  public:
    ~DelegateMultiCast()
    {
        clear();
    }

    template <typename ObjectClass_T> void add_object(ObjectClass_T* inObjPtr, void (ObjectClass_T::*inFunc)(Args_T...))
    {
        functions.push_back(std::make_shared<DelegateFunctionPtr<ObjectClass_T, void, Args_T...>>(inObjPtr, inFunc));
    }

    template <typename Lambda_T> void add_lambda(const Lambda_T& lambda)
    {
        lambda_expressions.emplace_back(std::make_shared<TLambdaClassStorage<Lambda_T, Args_T...>>(lambda));
    }

    void clear()
    {
        functions.clear();
        lambda_expressions.clear();
    }

    void clear_object(void* ObjPtr)
    {
        for (int64_t i = functions.size() - 1; i >= 0; --i)
        {
            if (*functions[i] == ObjPtr)
                functions.erase(functions.begin() + i);
        }
    }

    void execute(Args_T... inArgs)
    {
        for (const auto& fct : functions)
        {
            fct->execute(inArgs...);
        }
        for (const auto& lambda : lambda_expressions)
        {
            lambda->execute(std::forward<Args_T>(inArgs)...);
        }
    }

  private:
    std::vector<std::shared_ptr<DelegateFunctionPtrWrapper<void, Args_T...>>> functions;
    std::vector<std::shared_ptr<ILambdaClassStorage<Args_T...>>>              lambda_expressions;
};