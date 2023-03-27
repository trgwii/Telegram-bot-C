#define iterate_object(value, name)                                            \
  for (json_object_element_t *name =                                           \
           (assert(value->type == json_type_object),                           \
            ((json_object_t *)(value->payload))->start);                       \
       name; name = name->next)

#define iterate_array(value, name)                                             \
  for (json_array_element_t *name =                                            \
           (assert(value->type == json_type_array),                            \
            ((json_array_t *)(value->payload))->start);                        \
       name; name = name->next)

#define get_string(value)                                                      \
  (assert(value->type == json_type_string),                                    \
   ((json_string_t *)(value->payload))->string)

#define get_long(value)                                                        \
  (assert(value->type == json_type_number),                                    \
   strtol(((json_number_t *)(value->payload))->number, NULL, 10))

#define get_ulong(value)                                                       \
  (assert(value->type == json_type_number),                                    \
   strtoul(((json_number_t *)(value->payload))->number, NULL, 10))
