
////////////////////////////AUTOMATIC WRAP//////
  WRAPPED2 (bool,AutoPilotTo,UnitWrapper,un,bool,ignore_friendlies,false)
  voidWRAPPED0(SetTurretAI)
  voidWRAPPED0(DisableTurretAI)
  voidWRAPPED3(leach,float,XshieldPercent,float,YrechargePercent,float,ZenergyPercent)
  WRAPPED0 (int,getFgSubnumber,-1)
  WRAPPED0 (const string,getFgID,string())
  voidWRAPPED1(setFullname,string,name)
  WRAPPED0(string,getFullname,string())
  WRAPPED0(string,getFullAIDescription,string())
  voidWRAPPED3(setTargetFg,string,primary,string,secondary,string,tertiary)
  voidWRAPPED1(ReTargetFg,int,which_target)
  WRAPPED0(bool,isStarShip,false)
  WRAPPED0(bool,isPlanet,false)
  WRAPPED0(bool,isJumppoint,false)
  WRAPPED1(bool,isEnemy,UnitWrapper,other,false)
  WRAPPED1(bool,isFriend,UnitWrapper,other,false)
  WRAPPED1(bool,isNeutral,UnitWrapper,other,false)
  WRAPPED1(float,getRelation,UnitWrapper,other,0)
  voidWRAPPED1(ToggleWeapon,bool,Missile)
  voidWRAPPED1(SelectAllWeapon,bool,Missile)
  voidWRAPPED1( Split ,int, level)

  voidWRAPPED0(Init)
  voidWRAPPED1( ActivateJumpDrive ,int ,destination)
  voidWRAPPED0( DeactivateJumpDrive )
  voidWRAPPED0( Destroy)
  WRAPPED1(Vector,LocalCoordinates,UnitWrapper,un,Vector(0,0,0))
  WRAPPED3(bool,InRange,UnitWrapper,target, bool, cone, bool, cap,false)
  WRAPPED0 (float,CloakVisible,false)
  voidWRAPPED1 (Cloak,bool,cloak)
  voidWRAPPED0(RemoveFromSystem)
  WRAPPED2(QVector, PositionITTS ,QVector, local_posit, float, speed,QVector(0,0,0))
  WRAPPED0(QVector,Position,QVector(0,0,0))
  WRAPPED0(QVector, LocalPosition,QVector(0,0,0))

  WRAPPED0(UnitWrapper, Threat,UnitWrapper(0))
  voidWRAPPED1( TargetTurret ,UnitWrapper,targ)
  WRAPPED0(UnitCollection::UnitIterator, getSubUnits,0)
  voidWRAPPED2( Threaten ,UnitWrapper,targ, float,danger)
  voidWRAPPED0( ResetThreatLevel)
  voidWRAPPED1( Fire,bool,Missile)
  voidWRAPPED0( UnFire)
  WRAPPED0(float, computeLockingPercent,0)//how locked are we
  WRAPPED0(float, FShieldData,0)
  WRAPPED0(float, RShieldData,0)
  WRAPPED0(float, LShieldData,0)
  WRAPPED0(float, BShieldData ,0)
  WRAPPED0(float, FuelData,0)
  WRAPPED0(float, EnergyData,0)
  WRAPPED0(float, GetHull,0)
  WRAPPED0(float, rSize ,0)
  WRAPPED1(double, getMinDis,QVector,pnt,0)//for clicklist
  WRAPPED3(float, querySphere, QVector,start, QVector ,end, float ,my_unit_radius,0)
  WRAPPED3(int, queryBoundingBox,QVector,origin,Vector,direction, float, err,0)
  voidWRAPPED0( PrimeOrders)
  voidWRAPPED1( LoadAIScript, const std::string &,aiscript)
  WRAPPED0(bool, LoadLastPythonAIScript,false )
  WRAPPED0(bool, EnqueueLastPythonAIScript,false )
  voidWRAPPED1( SetPosition,QVector,pos)
  voidWRAPPED1( SetCurPosition ,QVector, pos)
  voidWRAPPED1( SetPosAndCumPos ,QVector,pos)
  voidWRAPPED1( Rotate,Vector,axis)
  voidWRAPPED1( ApplyForce,Vector,Vforce) 
  voidWRAPPED1( ApplyLocalForce,Vector,Vforce) 
  voidWRAPPED1( Accelerate,Vector,Vforce) 
  voidWRAPPED2( ApplyTorque ,Vector,Vforce, QVector,Location)
  voidWRAPPED2( ApplyBalancedLocalTorque ,Vector,Vforce, Vector,Location) 
  voidWRAPPED1( ApplyLocalTorque,Vector,torque) 
  WRAPPED2(float, DealDamageToHull ,Vector,pnt, float, Damage,0)
  WRAPPED2(Vector, ClampThrust,Vector,thrust, bool, afterburn,Vector(0,0,0))
  voidWRAPPED2( Thrust,Vector,amt,bool, afterburn )
  voidWRAPPED1( LateralThrust,float, amt)
  voidWRAPPED1( VerticalThrust,float, amt)
  voidWRAPPED1( LongitudinalThrust,float, amt)
  WRAPPED2(Vector, ClampVelocity ,Vector, velocity, bool, afterburn,Vector(0,0,0))
  WRAPPED1(Vector, ClampAngVel ,Vector, vel,Vector(0,0,0))
  WRAPPED1(Vector, ClampTorque,Vector,torque,Vector(0,0,0))
  voidWRAPPED2( SetOrientation ,QVector ,q, QVector, r)
  WRAPPED1(Vector, UpCoordinateLevel,Vector,v,Vector(0,0,0))
  WRAPPED1(Vector, DownCoordinateLevel,Vector,v,Vector(0,0,0))
  WRAPPED1(Vector, ToLocalCoordinates,Vector,v,Vector(0,0,0))
  WRAPPED1(Vector, ToWorldCoordinates,Vector,v,Vector(0,0,0))
  WRAPPED0(Vector,GetAngularVelocity,Vector(0,0,0))
  WRAPPED0(Vector,GetVelocity,Vector(0,0,0))
  voidWRAPPED1( SetVelocity ,Vector, v)
  voidWRAPPED1( SetAngularVelocity ,Vector, v)
  WRAPPED0(float, GetMoment,0)
  WRAPPED0(float, GetMass,0)
  WRAPPED0(int,LockMissile,0)
  voidWRAPPED1(EjectCargo,int,index)
  WRAPPED1(float,PriceCargo,const std::string &,s,0)
  WRAPPED1(Cargo &,GetCargo,int,i,Cargo())
  WRAPPED0(int,numCargo,0)
  WRAPPED1(bool,IsCleared,UnitWrapper,dockingunit,false)
  voidWRAPPED5(ImportPartList,const std::string&,category, float, price, float, pricedev, float, quantity, float, quantdev)
  WRAPPED1(bool,RequestClearance,UnitWrapper,dockingunit,false)
  WRAPPED1(bool,isDocked,UnitWrapper,dockingUnit,false)
  WRAPPED1(bool,Dock,UnitWrapper,unitToDockWith,false)
  WRAPPED1(bool,UnDock,UnitWrapper,unitToDockWith,false)
  WRAPPED0(int,GetNumMounts,0)

  EXPORT_UTIL(JumpTo,false)
  EXPORT_UTIL(getFactionName,'')
  EXPORT_UTIL(getFactionIndex,0)
  voidEXPORT_UTIL(setFactionName)
  voidEXPORT_UTIL(setFactionIndex)
  EXPORT_UTIL(getName,'')
  EXPORT_UTIL(getFlightgroupName,'')
  EXPORT_UTIL(getFgDirective,'')
  EXPORT_UTIL(getFlightgroupLeader,Unit)
  voidEXPORT_UTIL(addCredits)
  voidEXPORT_UTIL(switchFg)
  EXPORT_UTIL(getCredits,0)
  EXPORT_UTIL(setFlightgroupLeader,false)
  EXPORT_UTIL(setFgDirective,false)
  EXPORT_UTIL(getFgSubnumber,-1)
  EXPORT_UTIL(isSignificant,false)
  EXPORT_UTIL(isSun,false)
  EXPORT_UTIL(communicateTo,0)
  EXPORT_UTIL(commAnimation,false)
  EXPORT_UTIL(removeCargo,0)
  EXPORT_UTIL(upgrade,0)
  EXPORT_UTIL(addCargo,0)
  EXPORT_UTIL(incrementCargo,false)
  EXPORT_UTIL(decrementCargo,false)

