#include <ek/log.h>
#include <ek/app_native.h>

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>

@interface Billing : NSObject <SKProductsRequestDelegate, SKPaymentTransactionObserver> {
    NSMutableDictionary* _products;
    NSMutableDictionary* _transactionsDict;
    NSMutableArray* _transactionsQueue;
    BOOL _ready;
}
@end

@implementation Billing

- (id)init {
    self = [super init];

    _products = [NSMutableDictionary dictionary];
    _transactionsDict = [NSMutableDictionary dictionary];
    _transactionsQueue = [NSMutableArray array];
    _ready = false;

    [[SKPaymentQueue defaultQueue] addTransactionObserver:self];

    return self;
}

- (void)free {
    _ready = false;
}


- (void)updateProducts:(NSArray*)items {
    SKProductsRequest* productsRequest = [[SKProductsRequest alloc]
            initWithProductIdentifiers:[NSSet setWithArray:items]];
    productsRequest.delegate = self;
    [productsRequest start];
}


- (void)handleQueue:(SKPaymentQueue*)queue
updatedTransactions:(NSArray*)transactions {
    for (SKPaymentTransaction* transaction in transactions) {

        const char* trID = transaction.transactionIdentifier ? transaction.transactionIdentifier.UTF8String : "";
        const char* prodID = transaction.payment.productIdentifier.UTF8String;
        log_info("transaction %li '%s' '%s'", transaction.transactionState, trID, prodID);

        switch (transaction.transactionState) {
            // Call the appropriate custom method for the transaction state.
            case SKPaymentTransactionStatePurchasing:
                break;

            case SKPaymentTransactionStateDeferred:
                break;

            case SKPaymentTransactionStateFailed: {
                [queue finishTransaction:transaction];

                purchase_data_t purchase;
                purchase.product_id = prodID;
                purchase.token = trID;
                purchase.error_code = (int) transaction.error.code;
                purchase.state = -1; // failed
                // SKErrorPaymentCancelled
                if (transaction.error.code == SKErrorPaymentCancelled) {
                } else {
                }
                g_billing.on_purchase_changed(&purchase);
            }
                break;

            case SKPaymentTransactionStatePurchased: {
                [_transactionsDict setObject:transaction forKey:transaction.transactionIdentifier];

                purchase_data_t purchase;
                purchase.product_id = prodID;
                purchase.token = trID;
                purchase.state = 0; // purchased

                //                NSString* str = [[NSString alloc] initWithData:transaction.transactionReceipt encoding:NSUTF8StringEncoding];
                //                NSData *dataReceipt = [NSData dataWithContentsOfURL:[[NSBundle mainBundle] appStoreReceiptURL]];
                //                NSString *receipt = [dataReceipt base64EncodedStringWithOptions:0];
                //                purchase.receipt = [str UTF8String];

                NSString* userData = transaction.payment.applicationUsername;
                if (userData) {
                    purchase.payload = [userData UTF8String];
                }

                g_billing.on_purchase_changed(&purchase);
            }
                break;

            case SKPaymentTransactionStateRestored:
                break;

            default:
                break;
        }
    }
}


- (void)paymentQueue:(SKPaymentQueue*)queue
 updatedTransactions:(NSArray*)transactions {
    if (_ready) {
        [self handleQueue:queue updatedTransactions:transactions];
    } else {
        [_transactionsQueue addObjectsFromArray:transactions];
    }
}

- (void)ready {
    _ready = true;
}

- (void)restore {

    if (!_ready)
        return;

    NSArray* all = [_transactionsDict allValues];
    NSArray* purchased = [all filteredArrayUsingPredicate:[NSPredicate predicateWithBlock:^BOOL(id object,
                                                                                                NSDictionary* bindings) {
        return ((SKPaymentTransaction*) object).transactionState == SKPaymentTransactionStatePurchased;
    }]];

    [self handleQueue:[SKPaymentQueue defaultQueue] updatedTransactions:purchased];
    [self handleQueue:[SKPaymentQueue defaultQueue] updatedTransactions:_transactionsQueue];
    [_transactionsQueue removeAllObjects];

    [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
}

- (void)productsRequest:(SKProductsRequest*)request
     didReceiveResponse:(SKProductsResponse*)response {

    //NSArray *products = [response products];
    //NSArray *inv = response.invalidProductIdentifiers;

    //for (NSString *invalidIdentifier in response.invalidProductIdentifiers) {
    // Handle any invalid product identifiers.
    //}

    for (SKProduct* product in response.products) {
        [_products setObject:product forKey:product.productIdentifier];
        product_details_t details;
        details.product_id = [product.productIdentifier UTF8String];

        NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
        numberFormatter.numberStyle = NSNumberFormatterCurrencyStyle;
        numberFormatter.locale = product.priceLocale;
        NSString* formattedString = [numberFormatter stringFromNumber:product.price];
        details.price = [formattedString UTF8String];
        details.currency_code = [numberFormatter.currencyCode UTF8String];

        g_billing.on_product_details(&details);
    }
}

- (SKProduct*)getProduct:(const char*)name {
    NSString* str = [NSString stringWithUTF8String:name];

    return _products[str];
}

- (void)purchase:(const char*)prod
        :(const char*)payload {
    SKProduct* product = [self getProduct:prod];
    if (!product)
        return;

    SKMutablePayment* payment = [SKMutablePayment paymentWithProduct:product];
    payment.quantity = 1;
    payment.applicationUsername = [NSString stringWithUTF8String:payload];

    [[SKPaymentQueue defaultQueue] addPayment:payment];
}

- (void)consume:(const char*)token {
    NSString* str = [NSString stringWithUTF8String:token];

    SKPaymentTransaction* trans = _transactionsDict[str];
    if (!trans)
        return;

    [[SKPaymentQueue defaultQueue] finishTransaction:trans];
    [_transactionsDict removeObjectForKey:trans.transactionIdentifier];
}

@end

Billing* _billing = nullptr;

void billing_setup(const char* developer_key) {
    log_debug("billing setup");
    (void) developer_key;
    if (_billing != nil) {
        return;
    }
    _billing = [Billing new];
}

void billing_get_purchases() {
    if (_billing == nil) {
        return;
    }
    [_billing ready];
    [_billing restore];
}

void billing_get_details(const char** product_ids, uint32_t count) {
    if (_billing == nil) {
        return;
    }

    NSArray* array = [[NSArray alloc] init];

    for (uint32_t i = 0; i < count; ++i) {
        const char* product_id = product_ids[i];
        NSString* str = [NSString stringWithUTF8String: product_id];
        array = [array arrayByAddingObject:str];
    }

    [_billing updateProducts:array];
}

void billing_purchase(const char* product_id, const char* payload) {
    if (_billing == nil) {
        return;
    }

    [_billing purchase:product_id :payload];
}

void consume(const char* token) {
    if (_billing == nil) {
        return;
    }

    [_billing consume:token];
}
