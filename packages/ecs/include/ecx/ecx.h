#ifndef ECX_H
#define ECX_H

#include <ek/sparse_array.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void ecx_setup(void);

void ecx_shutdown(void);

enum {
    ECX_COMPONENTS_MAX_COUNT = 128,
    ECX_COMPONENT_DATA_LAYERS_MAX_COUNT = 2,
    ECX_ENTITIES_MAX_COUNT = 0x10000,
    ECX_LISTENERS_MAX_COUNT = 8,
};

typedef uint32_t entity_id_t;

typedef uint16_t entity_idx_t;
typedef uint8_t entity_gen_t;

typedef uint16_t component_handle_t;
typedef uint8_t component_type_id;

/**
 * The main entity structure. `id` is structured by Index and Generation.
 * @note Acts like weak pointer to Entity, use `is_entity` function to check if entity is valid and alive.
 */
typedef struct entity_t {
    union {
        /**
         * For fast comparing between entities
         */
        entity_id_t id;
        struct {
            entity_idx_t idx;
            entity_gen_t gen;
        };
    };
} entity_t;

#ifdef __cplusplus
#define NULL_ENTITY (entity_t{0u})
#else
#define NULL_ENTITY ((entity_t){0u})
#endif

/**
 * Constructs entity structure by known numerical ID
 * @param id entity's id
 * @return entity structure
 */
entity_t entity_id(entity_id_t id);

/**
 * Get current entity ID by global entity index, that means you could generate valid entity if index currently alive
 * @param idx - entity index
 * @return valid entity if index is currently used, otherwise returns invalid entity (not means that it will be NULL_ENTITY, but `is_entity` SHOULD returns `false`)
 */
entity_t entity_at(entity_idx_t idx);

/**
 * Check if entity alive and valid. For `NULL_ENTITY` (`id` is zero) entity is always invalid.
 * @param entity to check
 * @return entity is invalid or NULL
 */
bool is_entity(entity_t entity);

entity_t create_entity(void);

/**
 * remove all components from entity
 * @param entity to remove all compoennts from
 */
void clear_entity(entity_t entity);

/**
 * destroy entity (remove all components as well)
 * @param entity to destroy
 */
void destroy_entity(entity_t entity);

typedef struct {
    component_type_id prev;
    component_type_id next;
} ecx_component_list_t;

typedef struct {
    const char* label;
    uint16_t capacity;
    uint16_t data_num;
    uint16_t data_stride[ECX_COMPONENT_DATA_LAYERS_MAX_COUNT];
} ecx_component_type_decl;

typedef struct {
    ek_sparse_array entity_to_handle; // 0
    entity_idx_t* handle_to_entity; // 4
    // entities associated with this component, from 1 to MAX_ENTITIES,
    // 0 - is not initialized, because required at least zero-handle is allocated
    uint16_t size; // 8
    uint16_t data_stride[ECX_COMPONENT_DATA_LAYERS_MAX_COUNT]; // 10 .. 14
    uint16_t data_num; // 14

    void* data[ECX_COMPONENT_DATA_LAYERS_MAX_COUNT]; // 16 .. 24
    component_type_id* component_next; // 24
    void (* on_shutdown)(void); // 28
    void (* ctor)(component_handle_t handle); // 32
    void (* dtor)(component_handle_t handle); // 36

    // debug section
    const char* label; // 40
    /**
     * array index in global components(types) array obtained on registration,
     * used for linking enabled components for entity
     */
    uint16_t index; // 44
    uint16_t lock_counter; // 46
} ecx_component_type;

// C-API

/**
 * Removes component from entity, nothing if component is not set
 * @param type component type storage
 * @param entity target to remove component from
 * @return true if component has been removed, false otherwise (not set or entity is invalid in release mode)
 */
bool remove_component(ecx_component_type* type, entity_t entity);

/**
 *
 * @param type component storage
 * @param entity target
 * @return NULL if component is not set, assert/NULL if invalid entity
 */
void* get_component(ecx_component_type* type, entity_t entity);
void* get_component_n(ecx_component_type* type, entity_t entity, uint32_t index);
void* get_component_or_default(ecx_component_type* type, entity_t entity);
/**
 * Adds component to entity if is not set,
 * if entity has component - returns it, otherwise creates new data and set to entity), if entity is invalid - assert and returns NULL
 * @return pointer to #0 data, should be caster to component data type. If data #0 is not set (empty data component), returns just not NULL value. Asserts if entity is invalid or returns NULL
 */
void* add_component(ecx_component_type* type, entity_t entity);

/**
* Entity Pool generates dense indices for Entity representation.
* We track Generations on reusing deleted entities.
* Generation is 8-bit, so I swear to check references next update.
* Collision is possible if in one frame we do destroy and create more than 256 times
*/

typedef struct ecx_world_t {
    // allocated entities count
    uint32_t size;

    // next free index in entity pool
    uint32_t next;

    uint32_t components_num;

    uint32_t on_entity_destroy_num;
} ecx_world_t;

// entity pool data, indices and generations
// entity indices
extern entity_t ecx_pool[ECX_ENTITIES_MAX_COUNT]; // 4 * ENTITIES_MAX_COUNT = 262 kb
// first component per entity
extern component_type_id ecx_component_head[ECX_ENTITIES_MAX_COUNT];
// per component, data manager
extern ecx_component_type* ecx_components[ECX_COMPONENTS_MAX_COUNT];

typedef void (* on_entity_destroy)(entity_t e);
extern on_entity_destroy ecx_on_entity_destroy[ECX_LISTENERS_MAX_COUNT];

extern ecx_world_t ecx;

void foreach_entity(void(* callback)(entity_t));

void foreach_type(ecx_component_type* type, void(* callback)(component_handle_t));

// private
void* get_component_data(ecx_component_type* type, component_handle_t handle, uint32_t data_index);

component_handle_t get_component_handle_by_index(const ecx_component_type* type, entity_idx_t entity_idx);

component_handle_t get_component_handle(const ecx_component_type* type, entity_t entity);
entity_t get_entity(const ecx_component_type* type, component_handle_t handle);

/** internal functions **/

void init_component_type(ecx_component_type* type, ecx_component_type_decl decl);

void ecx_sort_component_type_table(ecx_component_type** types, uint32_t count);

component_handle_t ecx_create_component(ecx_component_type* type, entity_idx_t entity_idx);

#ifndef NDEBUG
#define ECX_TYPE_LABEL__(Type) #Type
#else
#define ECX_TYPE_LABEL__(Type) ""
#endif

#define ECX_ID(Name) ecx_##Name
#define ECX_DEFINE_TYPE(Type) ecx_component_type ECX_ID(Type)
#define ECX_GET(T,e) ((T*)get_component(&ECX_ID(T),e))
#define ECX_ADD(T,e) ((T*)add_component(&ECX_ID(T),e))
#define ECX_HAS(T,e) (get_component_handle(&ECX_ID(T),e)!=0)
#define ECX_DEL(T,e) remove_component(&ECX_ID(T),e)

#define ECX_TYPE_DECL_0(Type,Cap) ((ecx_component_type_decl){ECX_TYPE_LABEL__(Type),(Cap),0,{0}})
#define ECX_TYPE_DECL_1(Type,Cap,Data1) ((ecx_component_type_decl){ECX_TYPE_LABEL__(Type),(Cap),1,{sizeof(Data1)}})
#define ECX_TYPE_DECL_2(Type,Cap,Data1,Data2) ((ecx_component_type_decl){ECX_TYPE_LABEL__(Type),(Cap),2,{sizeof(Data1),sizeof(Data2)}})

#define ECX_TYPE_0(T,InitialCount) init_component_type(&ECX_ID(T),ECX_TYPE_DECL_0(T,InitialCount))
#define ECX_TYPE_1(T,InitialCount) init_component_type(&ECX_ID(T),ECX_TYPE_DECL_1(T,InitialCount,T))
#define ECX_TYPE_2(T,T2,InitialCount) init_component_type(&ECX_ID(T),ECX_TYPE_DECL_2(T,InitialCount,T,T2))
#define ECX_TYPE(T,InitialCount) ECX_TYPE_1(T,InitialCount)

#define ECX_FOR__(T, I) for (int I = 1; I < ECX_ID(T).size; ++I)
#define ECX_FOR_DRAFT(T) ECX_FOR__(T,h_##T)

#ifdef __cplusplus
}
#endif

#endif // ECX_H
