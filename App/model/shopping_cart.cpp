#include "shopping_cart.h"

#include <cmath>
#include <cstddef>

namespace
{
bool pricesMatch(double left, double right)
{
    return std::fabs(left - right) < 0.0001;
}
}

ShoppingCart::ShoppingCart() {}

bool ShoppingCart::addItem(const Souvenir& souvenir, int quantity)
{
    if (quantity <= 0)
        return false;

    for (CartItem& cart_item : _items)
    {
        const bool same_souvenir_id =
            souvenir.souvenir_id > 0 &&
            cart_item.item_souvenir.souvenir_id == souvenir.souvenir_id;
        const bool same_snapshot_identity =
            cart_item.item_souvenir.owner_stadium_id == souvenir.owner_stadium_id &&
            cart_item.item_souvenir.name == souvenir.name &&
            pricesMatch(cart_item.item_souvenir.price, souvenir.price);

        if (!same_souvenir_id && !same_snapshot_identity)
            continue;

        // Preserve the original purchase price in the cart. If the admin updates
        // a souvenir price mid-trip, a later purchase should become a new line item
        // instead of silently rewriting the earlier purchases to the new price.
        if (!same_snapshot_identity)
            continue;

        if (same_souvenir_id || same_snapshot_identity)
        {
            cart_item.quantity += quantity;
            return true;
        }
    }

    CartItem new_item;
    new_item.item_souvenir = souvenir;
    new_item.quantity = quantity;
    _items.push_back(new_item);
    return true;
}

bool ShoppingCart::removeItemAt(size_t index)
{
    if (index >= _items.size())
        return false;

    _items.erase(_items.begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}

void ShoppingCart::clear()
{
    _items.clear();
}

const std::vector<CartItem>& ShoppingCart::items() const
{
    return _items;
}

bool ShoppingCart::empty() const
{
    return _items.empty();
}

int ShoppingCart::totalQuantity() const
{
    int total = 0;

    for (const CartItem& cart_item : _items)
        total += cart_item.quantity;

    return total;
}

int ShoppingCart::totalQuantityForStadium(int stadium_id) const
{
    int total = 0;

    for (const CartItem& cart_item : _items)
    {
        if (cart_item.item_souvenir.owner_stadium_id == stadium_id)
            total += cart_item.quantity;
    }

    return total;
}

double ShoppingCart::totalCost() const
{
    double total = 0.0;

    for (const CartItem& cart_item : _items)
        total += cart_item.item_souvenir.price * cart_item.quantity;

    return total;
}

double ShoppingCart::totalCostForStadium(int stadium_id) const
{
    double total = 0.0;

    for (const CartItem& cart_item : _items)
    {
        if (cart_item.item_souvenir.owner_stadium_id == stadium_id)
            total += cart_item.item_souvenir.price * cart_item.quantity;
    }

    return total;
}
