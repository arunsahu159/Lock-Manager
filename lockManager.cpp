/* 
Goal : Build a lock manager. 
The lock manager should support the following capabilities:
1. Lock a resource in either shared or exclusive mode. 
2. Unlock a resource held by a transaction. 
A resource will be identified by a 10 character string. 
*/

#include <iostream>
#include <unordered_map>
#include <list>

using namespace std;

// Abstraction of a resource that can be locked. 
// A resource is locked in a 'mode' by a 'transaction'. 
// The lock request may be granted or put on wait based
// on a lock compatibility matrix. 
enum lockType
{
  SHARED,
  EXCLUSIVE 
};
enum lockStatus
{
  GRANTED,
  WAITING
};
class lockable_resource
{
private:
  int txn_id_;
  lockType lock_type_; // SHARED, EXCLUSIVE
  lockStatus lock_status_; // GRANTED, WAITING
public:
  // Constructor
  lockable_resource(int txn_id, lockType lock_type,lockStatus lock_status):txn_id_(txn_id),lock_type_(lock_type),lock_status_(lock_status)
  {
  }

  lockType getLockType() 
  {
    return(lock_type_);
  }

  lockStatus getStatus()
  {
    return(lock_status_);
  }

  int getTxnId()
  {
    return(txn_id_);
  }

  void setLockStatus(lockStatus st)
  {
    lock_status_ = st;
  }
};


// Resource
// type of lock
// txnid

lockStatus lock(std::string resource_name, int txn_id,lockType lock_type);

bool unlock(std::string resource_name,int txn_id);

unordered_map<std::string, list<lockable_resource > > lock_table;
void prnt(std:: string resource_name)
{
  list<lockable_resource> lst = lock_table[resource_name];
   std::list<lockable_resource>::iterator iter;
    for(iter=lst.begin();iter!=lst.end();++iter)
    {
      int txn_id = iter->getTxnId();
      lockType lt = iter->getLockType();
      lockStatus ls = iter->getStatus();
      cout<<"Transaction ID:"<<txn_id<<"Locktype:"<<lt<<" "<<"LockStatus:"<<ls<<endl;
    }
}
int main()
{
  lockStatus ret = lock("AAA", 1234,EXCLUSIVE);
  ret = lock("AAA", 7894,SHARED);
  ret = lock("AAA", 4567,EXCLUSIVE);
  ret = lock("AAA", 7854,SHARED);
  prnt("AAA");
  cout<<"Unlocking Transaction"<<"AAA"<<endl;
  bool var = unlock("AAA",1234);
  prnt("AAA");
  return 0;

}


lockStatus lock(std::string resource_name,int txn_id,lockType lock_type)
{
  lockStatus retval = WAITING;
  // Check if lock exists. 
  // No: Add to map, create new list and add lockable_resource to list
  if (lock_table.find(resource_name) == lock_table.end())
  {
    // lock table does not exist. 
    //  => lockable resource has to be created. 
    //     list of lockable resources has to be created. 
    //     lock table should be updated with resource. 

    // Creating new lockable resource
    lockable_resource lr(txn_id,lock_type,GRANTED);
    retval = GRANTED;
    // Add the lockable resource to the list
    list<lockable_resource> lst;
    lst.emplace_back(lr);
    lock_table[resource_name] = lst;
    cout<<"First time added to the table"<<endl;
    return(retval);
  }
  // if the resource is there in lock_table
  else
  {
    cout<<"Already present "<<"Transaction id"<<txn_id<<endl;
    list<lockable_resource> lst = lock_table.at(resource_name);
    std::list<lockable_resource>::iterator iter;
    for(iter=lst.begin();iter!=lst.end();++iter)
    {
      // int txn_id = iter->getTxnId();
      lockType lt = iter->getLockType();
      lockStatus ls = iter->getStatus();

      // cout<<"Transaction ID:"<<txn_id<<"Locktype:"<<lt<<" "<<"LockStatus:"<<ls<<endl;
      /*
      1.check for lock compatability
      2.Grant if only if: Shared lock and asked for shared lock
      3. Shared:0
         Exclusive:1
      */
      if(!lt && !lock_type)  // shared and compatible
      // Need to grant access for asked lock
      {
        // cout<<"GRANTED"<<endl;
        retval = GRANTED;
        lockable_resource lr(txn_id,lock_type,retval);
        // Since the lock asked for is granted I am putting it in the front of the list
        lst.emplace_front(lr);
        lock_table[resource_name] = lst;
        /*
        NOTE: Starvation is possible in this case
        E.g: if a transaction in exclusive mode is waiting and before it so
        many shared mode transactions comes all will be granted but exclusive transaction will wait forever
        */
        return(retval);
      }
      else // Exclusive and Not Compatible locks
      {
        // cout<<"WAITING"<<endl;
        lockable_resource lr(txn_id,lock_type,retval);
        // Waiting Transactions are at end of the list
        lst.emplace_back(lr);
        lock_table[resource_name] = lst;
        return retval;
      }
    } 
  }
   return(retval);

}


bool unlock(std::string resource_name,int txn_id)
{
  // Do only if the resource is there in the lockTable
  if(lock_table.find(resource_name)!=lock_table.end())
  {
    // find the list of transactions asked for this particular resource
    list<lockable_resource> lst = lock_table.at(resource_name);
    std::list<lockable_resource>::iterator iter;
    // Traverse the list
    // Delete the transaction for that particular resource
    for(iter=lst.begin();iter!=lst.end();++iter)
    {
      if(iter->getTxnId()==txn_id)
      {
        lst.erase(iter);
        break;
      }
    }
    /* The Way the Transactions are added in the list of lockable resource
    There are only two cases where the transactions status should be updated

    Case 1. The first transaction is in Exclusive State and Waiting, We need to update the
    transaction lockStatus->GRANTED
     */
    iter = lst.begin();
    if(iter->getLockType() && iter->getStatus())
    // Exclusive and Waiting
    {
      iter->setLockStatus(GRANTED);
    }
    /*
    Case 2: If the deleted transaction held a exclusive lock which means transactions
    after this were in waiting.
    so we need to traverse the list and give grant status for all compatible locks
    */
    else if(iter->getStatus()) //Granted: which means the transaction is in shared.
    // Grant access to all transactions asked for shared lock after this particular transaction.
    {
      for(iter=lst.begin();iter!=lst.end();)
      {
        if(!iter->getLockType()) // Shared lock: Grant it
        {
          iter->setLockStatus(GRANTED);
          auto temp = *iter;  //temporary variable to assign that particular transactions..
          iter = lst.erase(iter);
          lst.emplace_front(temp); // Put all the share node which is granted in front of the list..
          continue;
        }
        ++iter;
      }
    }
    lock_table[resource_name] = lst;
    // cout<<"Transactions succesfully deleted"<<endl;
    return true;
  }
  // cout<<"No resource found in the lock table"<<endl;
  return false;
}