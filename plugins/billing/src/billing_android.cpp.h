#include <ek/app_native.h>
#include <ek/log.h>

jobjectArray jniGetObjectStringArray(const char** strings, uint32_t count, JNIEnv* env) {
    jobjectArray res = env->NewObjectArray(count, env->FindClass("java/lang/String"), nullptr);

    for (size_t i = 0; i < count; ++i) {
        env->PushLocalFrame(1);
        env->SetObjectArrayElement(res, i, env->NewStringUTF(strings[i]));
        env->PopLocalFrame(0);
    }

    return res;
}

#define BILLING_CLASS_PATH "ek/billing/BillingBridge"

void billing_setup(const char* key) {
    log_debug("billing setup");

    JNIEnv* env = ek_android_jni();

    const char* method_name = "initialize";
    const char* method_sig = "(Ljava/lang/String;)V";

    auto class_ref = env->FindClass(BILLING_CLASS_PATH);
    auto key_ref = env->NewStringUTF(key);

    auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
    env->CallStaticVoidMethod(class_ref, method, key_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(key_ref);
}

void billing_get_purchases() {
    JNIEnv* env = ek_android_jni();

    const char* method_name = "getPurchases";
    const char* method_sig = "()V";

    auto class_ref = env->FindClass(BILLING_CLASS_PATH);
    auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
    env->CallStaticVoidMethod(class_ref, method);

    env->DeleteLocalRef(class_ref);
}

void billing_get_details(const char** product_ids, uint32_t count) {
    JNIEnv* env = ek_android_jni();

    const char* method_name = "getDetails";
    const char* method_sig = "([Ljava/lang/String;)V";

    auto class_ref = env->FindClass(BILLING_CLASS_PATH);
    jobjectArray product_ids_ref = jniGetObjectStringArray(product_ids, count, env);

    auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
    env->CallStaticVoidMethod(class_ref, method, product_ids_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(product_ids_ref);
}

void billing_purchase(const char* product_id, const char* payload) {
    JNIEnv* env = ek_android_jni();

    const char* method_name = "purchase";
    const char* method_sig = "(Ljava/lang/String;Ljava/lang/String;)V";

    auto class_ref = env->FindClass(BILLING_CLASS_PATH);
    auto product_id_ref = env->NewStringUTF(product_id);
    auto payload_ref = env->NewStringUTF(payload);

    auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
    env->CallStaticVoidMethod(class_ref, method, product_id_ref, payload_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(product_id_ref);
    env->DeleteLocalRef(payload_ref);
}

void billing_consume(const char* token) {
    JNIEnv* env = ek_android_jni();

    const char* method_name = "consume";
    const char* method_sig = "(Ljava/lang/String;)V";

    auto class_ref = env->FindClass(BILLING_CLASS_PATH);
    auto token_ref = env->NewStringUTF(token);

    auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
    env->CallStaticVoidMethod(class_ref, method, token_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(class_ref);
}

extern "C" {

JNIEXPORT void JNICALL Java_ek_billing_BillingBridge_nativePurchase(JNIEnv *env, jclass,
                                                                    jstring product_id,
                                                                    jstring token,
                                                                    jint state,
                                                                    jstring payload,
                                                                    jstring signature,
                                                                    jint response_code) {
    if (g_billing.on_purchase_changed) {
        purchase_data_t data;
        data.product_id = product_id ? env->GetStringUTFChars(product_id, nullptr) : "";
        data.token = token ? env->GetStringUTFChars(token, nullptr) : "";
        data.payload = payload ? env->GetStringUTFChars(payload, nullptr) : "";
        data.signature = signature ? env->GetStringUTFChars(signature, nullptr) : "";
        data.state = state;
        data.error_code = response_code;

        g_billing.on_purchase_changed(&data);

        if (product_id) env->ReleaseStringUTFChars(product_id, data.product_id);
        if (token) env->ReleaseStringUTFChars(token, data.token);
        if (payload) env->ReleaseStringUTFChars(payload, data.payload);
        if (signature) env->ReleaseStringUTFChars(signature, data.signature);
    }
}

JNIEXPORT void JNICALL Java_ek_billing_BillingBridge_nativeDetails(JNIEnv *env, jclass,
                                                                   jstring product_id,
                                                                   jstring price,
                                                                   jstring currency_code) {
    if (g_billing.on_product_details) {
        product_details_t data;
        data.product_id = product_id ? env->GetStringUTFChars(product_id, nullptr) : "";
        data.price = price ? env->GetStringUTFChars(price, nullptr) : "";
        data.currency_code = currency_code ? env->GetStringUTFChars(currency_code, nullptr) : "";

        g_billing.on_product_details(&data);

        if (product_id) env->ReleaseStringUTFChars(product_id, data.product_id);
        if (price) env->ReleaseStringUTFChars(price, data.price);
        if (currency_code) env->ReleaseStringUTFChars(currency_code, data.currency_code);
    }
}

}

#undef BILLING_CLASS_PATH
