#ifndef SHOPPING_CART_H
#define SHOPPING_CART_H

#include "../../data_types.h"

#include <cstddef>
#include <vector>

class ShoppingCart
{
public:
    ShoppingCart();

    bool addItem(const Souvenir& souvenir, int quantity);
    bool removeItemAt(size_t index);
    void clear();

    const std::vector<CartItem>& items() const;

    bool empty() const;
    int totalQuantity() const;
    int totalQuantityForStadium(int stadium_id) const;
    double totalCost() const;
    double totalCostForStadium(int stadium_id) const;

private:
    std::vector<CartItem> _items;
};

#endif // SHOPPING_CART_H
