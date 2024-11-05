#include <rpc/rpc.h>
#include "chebyshev.h"

bool_t
xdr_input_data(XDR *xdrs, input_data *objp)
{
    if (!xdr_array(xdrs, (char **)&objp->A.A_val, &objp->A.A_len, ~0,
        sizeof(double), (xdrproc_t)xdr_double))
        return FALSE;
    if (!xdr_array(xdrs, (char **)&objp->b.b_val, &objp->b.b_len, ~0,
        sizeof(double), (xdrproc_t)xdr_double))
        return FALSE;
    if (!xdr_int(xdrs, &objp->size))
        return FALSE;
    if (!xdr_int(xdrs, &objp->max_iter))
        return FALSE;
    return TRUE;
}

bool_t
xdr_output_data(XDR *xdrs, output_data *objp)
{
    if (!xdr_array(xdrs, (char **)&objp->x.x_val, &objp->x.x_len, ~0,
        sizeof(double), (xdrproc_t)xdr_double))
        return FALSE;
    if (!xdr_double(xdrs, &objp->time))
        return FALSE;
    return TRUE;
}