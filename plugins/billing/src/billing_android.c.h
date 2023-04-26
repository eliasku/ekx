#include <ek/app_native.h>
#include <ek/log.h>

static jobjectArray jniGetObjectStringArray(const char** strings, uint32_t count, JNIEnv* env) {
    jobjectArray res = (*env)->NewObjectArray(env, count, (*env)->FindClass(env, "java/lang/String"), NULL);

    for (size_t i = 0; i < count; ++i) {
        (*env)->PushLocalFrame(env, 1);
        (*env)->SetObjectArrayElement(env, res, i, (*env)->NewStringUTF(env, strings[i]));
        (*env)->PopLocalFrame(env, 0);
    }

    return res;
}

#define BILLING_CLASS_PATH "ek/billing/BillingBridge"

void billing_setup(const char* key) {
    log_debug("billing setup");

    JNIEnv* env = ek_android_jni();

    const char* method_name = "initialize";
    const char* method_sig = "(Ljava/lang/String;)V";

    jclass class_ref = (*env)->FindClass(env, BILLING_CLASS_PATH);
    jstring key_ref = (*env)->NewStringUTF(env, key);

    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, method_name, method_sig);
    (*env)->CallStaticVoidMethod(env, class_ref, method, key_ref);

    (*env)->DeleteLocalRef(env, class_ref);
    (*env)->DeleteLocalRef(env, key_ref);
}

void billing_get_purchases() {
    JNIEnv* env = ek_android_jni();

    const char* method_name = "getPurchases";
    const char* method_sig = "()V";

    jclass class_ref = (*env)->FindClass(env, BILLING_CLASS_PATH);
    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, method_name, method_sig);
    (*env)->CallStaticVoidMethod(env, class_ref, method);

    (*env)->DeleteLocalRef(env, class_ref);
}

void billing_get_details(const char** product_ids, uint32_t count) {
    JNIEnv* env = ek_android_jni();

    const char* method_name = "getDetails";
    const char* method_sig = "([Ljava/lang/String;)V";

    jclass class_ref = (*env)->FindClass(env, BILLING_CLASS_PATH);
    jobjectArray product_ids_ref = jniGetObjectStringArray(product_ids, count, env);

    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, method_name, method_sig);
    (*env)->CallStaticVoidMethod(env, class_ref, method, product_ids_ref);

    (*env)->DeleteLocalRef(env, class_ref);
    (*env)->DeleteLocalRef(env, product_ids_ref);
}

void billing_purchase(const char* product_id, const char* payload) {
    JNIEnv* env = ek_android_jni();

    const char* method_name = "purchase";
    const char* method_sig = "(Ljava/lang/String;Ljava/lang/String;)V";

    jclass class_ref = (*env)->FindClass(env, BILLING_CLASS_PATH);
    jstring product_id_ref = (*env)->NewStringUTF(env, product_id);
    jstring payload_ref = (*env)->NewStringUTF(env, payload);

    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, method_name, method_sig);
    (*env)->CallStaticVoidMethod(env, class_ref, method, product_id_ref, payload_ref);

    (*env)->DeleteLocalRef(env, class_ref);
    (*env)->DeleteLocalRef(env, product_id_ref);
    (*env)->DeleteLocalRef(env, payload_ref);
}

void billing_consume(const char* token) {
    JNIEnv* env = ek_android_jni();

    const char* method_name = "consume";
    const char* method_sig = "(Ljava/lang/String;)V";

    jclass class_ref = (*env)->FindClass(env, BILLING_CLASS_PATH);
    jstring token_ref = (*env)->NewStringUTF(env, token);

    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, method_name, method_sig);
    (*env)->CallStaticVoidMethod(env, class_ref, method, token_ref);

    (*env)->DeleteLocalRef(env, class_ref);
    (*env)->DeleteLocalRef(env, class_ref);
}

JNIEXPORT void JNICALL Java_ek_billing_BillingBridge_nativePurchase(JNIEnv *env, jclass cls,
                                                                    jstring product_id,
                                                                    jstring token,
                                                                    jint state,
                                                                    jstring payload,
                                                                    jstring signature,
                                                                    jint response_code) {
    UNUSED(cls);
    if (g_billing.on_purchase_changed) {
        purchase_data_t data;
        data.product_id = product_id ? (*env)->GetStringUTFChars(env, product_id, NULL) : "";
        data.token = token ? (*env)->GetStringUTFChars(env, token, NULL) : "";
        data.payload = payload ? (*env)->GetStringUTFChars(env, payload, NULL) : "";
        data.signature = signature ? (*env)->GetStringUTFChars(env, signature, NULL) : "";
        data.state = state;
        data.error_code = response_code;

        g_billing.on_purchase_changed(&data);

        if (product_id) (*env)->ReleaseStringUTFChars(env, product_id, data.product_id);
        if (token) (*env)->ReleaseStringUTFChars(env, token, data.token);
        if (payload) (*env)->ReleaseStringUTFChars(env, payload, data.payload);
        if (signature) (*env)->ReleaseStringUTFChars(env, signature, data.signature);
    }
}

JNIEXPORT void JNICALL Java_ek_billing_BillingBridge_nativeDetails(JNIEnv *env, jclass cls,
                                                                   jstring product_id,
                                                                   jstring price,
                                                                   jstring currency_code) {
    UNUSED(cls);
    if (g_billing.on_product_details) {
        product_details_t data;
        data.product_id = product_id ? (*env)->GetStringUTFChars(env, product_id, NULL) : "";
        data.price = price ? (*env)->GetStringUTFChars(env, price, NULL) : "";
        data.currency_code = currency_code ? (*env)->GetStringUTFChars(env, currency_code, NULL) : "";

        g_billing.on_product_details(&data);

        if (product_id) (*env)->ReleaseStringUTFChars(env, product_id, data.product_id);
        if (price) (*env)->ReleaseStringUTFChars(env, price, data.price);
        if (currency_code) (*env)->ReleaseStringUTFChars(env, currency_code, data.currency_code);
    }
}

#undef BILLING_CLASS_PATH
