#ifndef BORROW_FUNCTOR_H
#define BORROW_FUNCTOR_H

#include <memory>
#include "Member.h"
#include "Book.h"
#include "Exceptions.h"

struct BorrowFunctor {
    std::shared_ptr<Member> member;

    explicit BorrowFunctor(std::shared_ptr<Member> m)
        : member(std::move(m)) {}

    void operator()(const std::shared_ptr<Media>& item) const {
        if (!member) throw GeneralFailure("No member provided");
        if (!item) throw GeneralFailure("No book provided");

        member->borrowItem(item);
    }
};

#endif