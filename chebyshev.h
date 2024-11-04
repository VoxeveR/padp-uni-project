/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _CHEBYSHEV_H_RPCGEN
#define _CHEBYSHEV_H_RPCGEN

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

struct result_data {
	struct {
		u_int solution_len;
		double *solution_val;
	} solution;
	double computation_time;
};
typedef struct result_data result_data;

struct matrix_data {
	struct {
		u_int matrix_len;
		double *matrix_val;
	} matrix;
	struct {
		u_int vector_len;
		double *vector_val;
	} vector;
	int size;
};
typedef struct matrix_data matrix_data;

#define CHEBYSHEV_PROG 0x20000001
#define CHEBYSHEV_VERS 1

#if defined(__STDC__) || defined(__cplusplus)
#define SOLVE_SYSTEM 1
extern  result_data * solve_system_1(matrix_data *, CLIENT *);
extern  result_data * solve_system_1_svc(matrix_data *, struct svc_req *);
extern int chebyshev_prog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define SOLVE_SYSTEM 1
extern  result_data * solve_system_1();
extern  result_data * solve_system_1_svc();
extern int chebyshev_prog_1_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_result_data (XDR *, result_data*);
extern  bool_t xdr_matrix_data (XDR *, matrix_data*);

#else /* K&R C */
extern bool_t xdr_result_data ();
extern bool_t xdr_matrix_data ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_CHEBYSHEV_H_RPCGEN */
