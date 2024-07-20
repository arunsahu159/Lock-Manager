# Lock-Manager

This repository is a simple demonstration of the lock manager in Database management systems for transactions.

The class `lockable_resource` is contains the details of the transaction which includes transaction id, the lock type the transaction is asking for, and the lock status of the particular transaction.

There is mapping between the resource and the transactions asking for that particular resource. if there are multiple transactions asking for the same resource then a list of the transacrtion is maintained. 

The lock status of the transactions are updated on the basis of the lock table. 

The repository also provide upgrade and downgrade of the lock on the resource.
