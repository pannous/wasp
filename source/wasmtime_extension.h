#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Check if an `anyref` is a `structref`.
 *
 * \param context the store context
 * \param anyref the anyref to check
 *
 * \return `true` if the `anyref` is a `structref`, `false` otherwise (including
 *         if the anyref is null)
 */
WASM_API_EXTERN bool
wasmtime_anyref_is_struct(wasmtime_context_t *context,
                          const wasmtime_anyref_t *anyref);

/**
 * \brief Convert an `anyref` to a `structref` if it is one.
 *
 * This function will check if the given `anyref` is a `structref` and if so,
 * write it to `out` as an anyref that can be used with struct operations.
 *
 * \param context the store context
 * \param anyref the anyref to check and convert
 * \param out where to store the converted struct reference (as anyref)
 *
 * \return `true` if the `anyref` is a `structref` and `out` has been filled,
 *         `false` otherwise
 */
WASM_API_EXTERN bool
wasmtime_anyref_as_struct(wasmtime_context_t *context,
                          const wasmtime_anyref_t *anyref,
                          wasmtime_anyref_t *out);

/**
 * \brief Get a field from a WebAssembly GC struct.
 *
 * This function reads the field at the given index from a struct reference.
 * The `anyref` must be a `structref`, otherwise this function returns `false`.
 *
 * Note that `i8` and `i16` field values are zero-extended into `i32` values.
 *
 * \param context the store context
 * \param anyref the struct reference (as anyref)
 * \param index the field index (0-based)
 * \param out where to store the field value
 *
 * \return `true` if successful and `out` has been filled, `false` if the
 *         anyref is not a structref or the index is out of bounds
 */
WASM_API_EXTERN bool
wasmtime_anyref_struct_get_field(wasmtime_context_t *context,
                                 const wasmtime_anyref_t *anyref,
                                 size_t index,
                                 wasmtime_val_t *out);

#ifdef __cplusplus
}
#endif
