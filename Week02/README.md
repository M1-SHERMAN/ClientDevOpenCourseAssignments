# 第二周作业：学习笔记 & 思路总结

![VideoLink](Video/Week02Video.mp4)

## 作业要求

基于FirstPerson示例，实现一个demo，具备以下功能：

（X、Y、N、T这些内容表示可以配置）
1. 规则
    1. 射击命中方块，获得积分X分
    2. 方块被子弹击中后，缩放为Y倍，再次被命中后销毁
2. 流程
    1. 游戏开始时随机N个方块成为重要目标，射击命中后获得双倍积分
    2. 游戏开始后限时T秒，时间到后游戏结算，打印日志输出每个玩家获得的积分和所有玩家获得的总积分
3. 附加题
    1. 利用UMG制作结算UI替代日志打印
    2. 支持多人联机


# 学习笔记 & 思路整理

特别鸣谢：Claude

参考文档：

- [虚幻引擎中的 RPC](https://dev.epicgames.com/documentation/zh-cn/unreal-engine/remote-procedure-calls-in-unreal-engine)
- [虚幻引擎网络概述](https://dev.epicgames.com/documentation/zh-cn/unreal-engine/networking-overview-for-unreal-engine#%E6%95%99%E7%A8%8B)
- [虚幻引擎多人游戏编程快速入门指南](https://dev.epicgames.com/documentation/zh-cn/unreal-engine/multiplayer-programming-quick-start-for-unreal-engine)
- [虚幻引擎中的 Actor 所有者和所属连接](https://dev.epicgames.com/documentation/zh-cn/unreal-engine/actor-owner-and-owning-connection-in-unreal-engine)
- [Delegates and Lamba Functions in Unreal Engine](https://dev.epicgames.com/documentation/zh-cn/unreal-engine/delegates-and-lamba-functions-in-unreal-engine)
- [UE5 中 Json 文件的读取与解析](https://zhuanlan.zhihu.com/p/545526337)

## 框架设计

1. 整体框架
   1. `AUELearnProjectGameMode`: 作为游戏的核心控制器，只关注游戏规则和流程，负责管理游戏规则、生成目标方块、处理计分等核心游戏逻辑。
   2. `AMyGameState`: 负责存储和同步游戏状态，比如剩余时间、特殊方块数量、玩家分数等需要在所有客户端之间保持一致的数据。
2. 玩家相关
   1. `AUELearnProjectCharacter`: 玩家角色类，处理玩家的移动、视角控制等基本操作。
   2. `AMyPlayerController`: 负责处理玩家输入和 UI 显示，比如游戏过程中的 HUD 和结束 UI。
3. 游戏玩法相关
   - `AShootingCubeBase`: 可击打目标的基类，实现了`IScorable`接口。
   - `UUELearnProjectWeaponComponent`: 武器组件，处理射击逻辑。
   - `AUELearnProjectProjectile`: 投射物类，处理子弹的物理运动和碰撞检测。

## 各个类的协同

```plain text
	     游戏开始
		↓
GameMode、GameState初始化（加载配置、设置游戏规则）
		↓
生成玩家角色(Character)
		↓
游戏循环开始：
  - GameMode控制方块的生成
  - GameState同步游戏状态
  - Character处理玩家输入
  - WeaponComponent处理射击
  - Projectile处理碰撞检测
  - IScorable接口处理得分
```

# **网络同步**

## **服务器权威性**

- 游戏采用了 UE 标准的服务器权威（Server Authority）架构
  - 服务器作为权威，负责处理所有关键的游戏逻辑和状态更新
  - 客户端主要负责显示和输入处理
- 通过网络复制（Replication）和 RPC（Remote Procedure Calls）实现服务器与客户端之间的通信

## 一些具体案例

1. 命中事件的处理

   当投射物击中目标时，处理流程如下：

   ```plain text
   客户端 Projectile::OnHit()
   		↓
   检查目标是否实现了IScorable接口
   		↓
   调用IScorable::HandleHitEvent()
   		↓
   Character::ServerReportHit() [Client -> Server RPC]
   		↓
   服务器端 ShootingCubeBase::ServerHandleHitEvent()
   		↓
   更新命中计数，调整大小
   		↓
   通过GameMode添加分数
   		↓
   GameState更新并复制新的分数到所有客户端
   ```

2. JSON 配置的网络同步

   遇到的问题：

   - 最开始直接在 GameState 的构造函数中读取 JSON 文件，并初始化相关变量。
   - 而网络游戏中，客户端的 GameState 是由服务器复制的。
   - 也就是说，构造函数可能在网络复制前就执行了，进而导致数据的不同步。

   解决方法：使用`PostInitializeComponents()`进行加载操作，因为这个函数会在所有组件初始化之后、游戏正式开始之前进行调用：

   ```cpp
   void AMyGameState::PostInitializeComponents()
   {
       Super::PostInitializeComponents();
       if (HasAuthority())
       {
           LoadConfig();
       }
   }

   // 注意：关键变量需要标记为Replicated来确保同步
   ```

   由此可以保证：

   - 配置只在服务器上加载一次
   - 通过 UE 的网络复制系统自动同步到所有客户端
   - 避免了配置不一致的问题

3. 投射物的可见性问题

   问题描述：在官方的第一人称模板中，客户端 A 的玩家没有办法看到，客户端 B 玩家发射出来的投射物。

   问题原因：

   - 这主要是因为 Projectile（弹丸）是由客户端直接生成的。
   - 而在 UE 的网络框架中，客户端生成的 Actor 默认不会在服务器上存在，因此无法被其他客户端感知。
   - 这导致了 Projectile 的状态无法正确复制到其他客户端，只能在本地客户端上看到自己生成的 Projectile。

   解决方法：

   - 在角色类中添加并实现 RPC 函数

     ```cpp
     // header
     UFUNCTION(Server, Reliable)
     void ServerReportFire(UUELearnProjectWeaponComponent* WeaponComponent)

     // cpp
     void AUELearnProjectCharacter::ServerReportFire_Implementation(UUELearnProjectWeaponComponent* WeaponComponent)
     {
         if (WeaponComponent != nullptr)
         {
             if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
             {
                 WeaponComponent->ServerHandleFire(PlayerController);
             }
         }
     }
     ```

   - 修改客户端的射击逻辑

     ```cpp
     void UUELearnProjectWeaponComponent::HandleFire()
     {
         if (Character == nullptr || Character->GetController() == nullptr)
         {
             return;
         }

         // 关键改动：不再直接生成投射物，而是通知服务器
         Character->ServerReportFire(this);

         // 这些是视觉效果，可以直接在客户端执行
         if (FireSound != nullptr)
         {
             UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
         }
         //...
     }
     ```

   - 确保 Projectile 类设置了复制

   修改前和修改后的对比：

   ```plain text
   修改前（原版模板）：
   ------------------------------------------------------------------------------
   客户端A                    服务器                     客户端B
      |                         |                         |
   按下射击                     |                         |
      |                        |                          |
   生成投射物                   |                         |
   播放音效                     |                         |
      |                         |                         |
   客户端A看到投射物            |                    客户端B看不到A的投射物
      |                         |                         |
   检测碰撞                     |                          |
      |                         |                         |
   ------------------------------------------------------------------------------

   修改后：
   ------------------------------------------------------------------------------
   客户端A                    服务器                     客户端B
      |                         |                         |
   按下射击                     |                         |
      |                         |                         |
   播放音效                     |                         |
      |                         |                         |
      |  ServerReportFire RPC   |                         |
      |------------------------>|                         |
      |                         |                         |
      |                     验证射击                      |
      |                         |                         |
      |                     生成投射物                    |
      |                         |                         |
      |      复制投射物         |       复制投射物         |
      |<------------------------|------------------------>|
      |                         |                         |
   客户端A看到投射物             |                    客户端B看到A的投射物
      |                         |                         |
      |                     检测碰撞                      |
      |                         |                         |
   ------------------------------------------------------------------------------
   ```

# 游戏配置文件读取

详见文章开头的参考文档：**UE5 中 Json 文件的读取与解析**

## 整体流程

1. 从路径中读取 JSON 文件
2. 检查 JSON 文件是否存在
3. 读取 JSON 文件内容
4. 解析 JSON 文件
5. 按照树形结构提取具体配置

```cpp
void AMyGameState::LoadConfig()
{
    // 1. 构造配置文件路径
    const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("GameConfig.json");

    // 2. 检查文件是否存在
    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*ConfigPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Config file not found: %s"), *ConfigPath);
        return;
    }

    // 3. 读取文件内容
    FString JsonString;
    if (FFileHelper::LoadFileToString(JsonString, *ConfigPath))
    {
        // 4. 解析JSON
        TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonString);
        TSharedPtr<FJsonObject> JsonObject;

        if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
        {
            // 5. 提取具体配置项
            if (JsonObject->HasField(TEXT("RemainingGameTime")))
            {
                RemainingGameTime = JsonObject->GetNumberField(TEXT("RemainingGameTime"));
            }
            // ... 其他配置项的读取
        }
    }
}
```

# 游戏玩法相关

## 方块`ShootingCube`类的设计 & 使用

### 身份标识：`IScorable`接口

```cpp
class UELEARNPROJECT_API IScorable
{
    GENERATED_BODY()
public:
    virtual void HandleHitEvent(AController* InstigatorController) = 0;
};
```

这个接口类似于一个身份标识，确保玩家发射的 Projectile 与 Actor 发生碰撞时，通过检测该 Actor 是否具有`IScorable`来决定是否需要执行得分的逻辑。

### `ShootingCubeBase`基类

这个基类中实现了基本的功能，主要负责管理自身的状态和视觉效果：

- 组件结构
  - `CollisionBox`：用于碰撞检测
  - `CubeMesh`：用于视觉展示
- 游戏逻辑的相关属性
  - `HitCounter`：记录被击中次数
  - `HitScore`：记录击中得分
  - `ScaledSize`：记录缩放大小
  - 需要注意：`HitScore`和`ScaledSize`是在 GameState 类中，通过解析一个 JSON 文件读取数据，然后在 GameMode 类中，当 Cube 实际被生成时才解析出来的数据被初始化的
- 关键行为：`HandleHitEvent()`与`ServerHandleHitEvent()`
  （关于击中的处理流程见网络同步）

  - `HandleHitEvent`
    - 接收一个参数`InstigatorController`：这是造成此次击中的控制器，通常是玩家控制器
    - 在这个方法中，需要检查：传入的`InstigatorController`是否有效，是否能获取`InstigatorController`所控制的`Pawn`，是否能将获取到的`Pawn`转换为具体的角色类。
    - 只有当这些检查全部完成时，才会调用角色类的`ServerReportHit`方法，将事件报告给玩家角色，由玩家角色调用服务器的 RPC 函数以处理击中事件
      _为什么这么做？一方面，客户端不可能拥有 Cube，无法直接在 Cube 上调用服务器 RPC 函数；另一方面，击中逻辑的处理属于游戏的关键逻辑，不能信任客户端的数据_
  - `ServerHandleHitEvent`

    - 主要处理具体的击中逻辑，通过 HitCounter 记录被击中的次数
    - 第一次击中：
      - 方块变大（通过 ScaledSize 控制）
      - 玩家得分（交给 GameMode 处理）
    - 第二次击中：

      - 玩家得分
      - 销毁方块

      ```cpp
      void AShootingCubeBase::ServerHandleHitEvent_Implementation(AController *InstigatorController)
      {
      	HitCounter++;
      	if (HitCounter == 1)
      	{
      		SetActorScale3D(GetActorScale3D() * ScaledSize);

      		if (AUELearnProjectGameMode *GameMode = Cast<AUELearnProjectGameMode>(GetWorld()->GetAuthGameMode()))
      		{
      			const int ScoreToAdd = HitScore;
      			GameMode->AddScore(InstigatorController, ScoreToAdd);
      		}
      	}
      	else if (HitCounter == 2)
      	{

      		if (AUELearnProjectGameMode *GameMode = Cast<AUELearnProjectGameMode>(GetWorld()->GetAuthGameMode()))
      		{
      			const int ScoreToAdd = HitScore;
      			GameMode->AddScore(InstigatorController, ScoreToAdd);
      			Destroy();
      		}
      	}
      }
      ```

### 方块的动态生成机制

这个机制主要由 GameMode 进行控制。

核心思路为：

1. 事先在场景中放置好目标点（TargetPoint）以确定在哪里生成方块
2. 在 GameMode 的 GenerateCubes 方法中，执行以下逻辑

   1. 获取并遍历所有在场景中的目标点

      ```cpp
      	TArray<AActor*> TargetPoints;
      	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), TargetPoints);

      	for (AActor* TargetPoint : TargetPoints)
      	{
      	  //...
      	}
      ```

   2. 从 GameState 中读取配置信息（HitScore、ScaledSize）（生成的范围 CubeSpawnRange 在 Beginplay 中读取）

      ```cpp
      		if (AMyGameState* MyGameState = GetGameState<AMyGameState>())
      		{
      			int ScoreToAdd = MyGameState->GetHitScore();
      			const float ScaledSize = MyGameState->GetScaledSize();
      			// ...
      		}
      ```

   3. 计算方块的生成位置（需要确保不会与已经生成的方块重叠）

      ```cpp
      				/* 计算Cube生成的位置和旋转 */
      				FVector Location;
      				// 需要确保生成的Cube不会与其他物体重叠，如果重叠则需要重新计算位置
      				bool bLocationSpawned = false;

      				// 设置最大尝试次数，避免当找不到Cube可以生成的位置时导致死循环
      				constexpr int MaxAttempts = 50;
      				int CurrentAttempt = 0;
      				while (!bLocationSpawned && CurrentAttempt <= MaxAttempts)
      				{

      					Location = TargetPointCasted->GetActorLocation();

      					// 在X轴上生成Cube，Y轴和Z轴的位置随机
      					Location.Y += FMath::RandRange(-CubeSpawnRange.Y, CubeSpawnRange.Y);
      					Location.Z += FMath::RandRange(-CubeSpawnRange.Z, CubeSpawnRange.Z);

      					// 忽略TargetPoint的碰撞
      					FCollisionQueryParams QueryParams;
      					QueryParams.AddIgnoredActor(TargetPointCasted);

      					// 使用OverlapAnyTestByChannel函数来检测生成的Cube是否与其他物体重叠
      					bLocationSpawned = !GetWorld()->OverlapAnyTestByChannel(
      						Location,										// 检测的位置
      						FQuat::Identity,								// 检测的旋转
      						ECollisionChannel::ECC_WorldDynamic,			// 检测的碰撞频道
      						FCollisionShape::MakeBox(FVector(50.0f)),	// 检测的形状
      						QueryParams);									// 检测的参数，这里忽略TargetPoint的碰撞

      					CurrentAttempt++;
      				}

      				// 如果当前TargetPoint没有找到合适的位置生成Cube，则继续下一个TargetPoint
      				if (!bLocationSpawned)
      				{
      					continue;
      				}
      ```

   4. 判断将要生成的方块的类型

      ```cpp
      				// 获取特殊方块的数量，如果还有特殊方块剩余，则有概率生成特殊方块
      				const int CurrentRemainingSpecialCubeNumber = MyGameState->GetRemainingSpecialCube();
      				TSubclassOf<AShootingCubeBase> CubeClass;
      				if (CurrentRemainingSpecialCubeNumber > 0)
      				{
      					CubeClass = FMath::RandBool() ? NormalCubeClass : SpecialCubeClass;
      					if (CubeClass == SpecialCubeClass)
      					{
      						// 如果生成了特殊方块，则需要减少剩余的特殊方块数量，并且将分数翻倍
      						MyGameState->SetRemainingSpecialCube(CurrentRemainingSpecialCubeNumber - 1);
      						ScoreToAdd *= 2;
      					}
      				}
      				else
      				{
      					CubeClass = NormalCubeClass;
      				}
      ```

   5. 生成方块，同时设置方块中 HitScore 和 ScaledSize 的值

      ```cpp
      				if (AShootingCubeBase* SpawnedCube = GetWorld()->SpawnActor<AShootingCubeBase>(CubeClass, Location, Rotation))
      				{
      					SpawnedCube->SetHitScore(ScoreToAdd);
      					SpawnedCube->SetScaledSize(ScaledSize);
      				}
      ```

## 计时系统

核心职责：管理游戏时间并在时间结束时触发相应的游戏逻辑。

在这里，主要通过 GameState 和 GameMode 两个类来实现：GameState 负责时间数据的存储与同步，而 GameMode 负责时间的更新

### 时间数据的存储与同步

在 GameState 中：

```cpp
UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_RemainingTime)
float RemainingGameTime;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeUpdated, float, NewTime);
UPROPERTY(BlueprintAssignable)
FOnTimeUpdated OnTimeUpdated;
```

这里，RemainingGameTime 使用了 RepNotify（属性复制通知），这可以自动跟踪属性值的变化，确保客户端接收到新值时自动触发回调函数。

### 时间的更新

在 GameMode 中：

1.  首先，设置一个定时器（在 BeginPlay 事件当中）

    ```cpp
    GetWorld()->GetTimerManager().SetTimer(
        GameTimerHandle,    // 定时器句柄
        this,              // 定时器所属对象
        &AUELearnProjectGameMode::UpdateGameTime,  // 回调函数
        1.0f,             // 调用间隔（秒）
        true              // 是否循环
    );
    ```

    这段代码在游戏开始时创建了一个循环定时器。为什么要用定时器而不是 Tick 函数呢？举个例子：

    想象你在做一个倒计时，有两种方式：

    1. 每帧都检查并更新时间（Tick 方式）
    2. 每秒触发一次更新（Timer 方式）

    如果游戏运行在 120 帧/秒，用 Tick 方式就会在 1 秒内进行 120 次检查，而 Timer 方式只需要 1 次。

    这就像要数 60 秒，你可以看 60 次手表，也可以一直盯着手表看——显然前者更有效率。

2.  时间更新的具体实现：`UpdateGameTime()` - **服务器权威性检查**
    `cpp
      if (!HasAuthority()) return;
      `
    这行代码确保时间只在服务器上更新。类似一个足球比赛，只有裁判（服务器）能决定正式时间，球员（客户端）只能查看时间。 - **时间更新和同步**
    `cpp
      RemainingGameTime = MyGameState->GetRemainingGameTime() - 1.0f;
      MyGameState->SetRemainingGameTime(RemainingGameTime);
      `
    这里通过 GameState 进行时间更新。在 GameState 中：
    `cpp
      void AMyGameState::SetRemainingGameTime(float NewTime)
      {
          if (HasAuthority())
          {
              RemainingGameTime = NewTime;
              OnTimeUpdated.Broadcast(RemainingGameTime);
          }
      }
      `
    当时间更新时，会触发`OnTimUpdated`事件，任何关心时间变化的系统都可以监听这个事件。 - **游戏结束处理**
    当时间用尽时，使用 Multicast RPC 确保所有客户端同时收到游戏结束的消息。
    `cpp
if (RemainingGameTime <= 0.0f)
{
    MyGameState->MulticastEndGame();
    GetWorld()->GetTimerManager().ClearTimer(GameTimerHandle);
}
`

### 时间变化的响应

1. 事件通知系统

   在 GameState 中，我们定义了两个关键的委托：

   ```cpp
   // 时间更新的委托，参数为新的时间值，主要用于HUD的更新
   DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeUpdated, float, NewTime);
   UPROPERTY(BlueprintAssignable)
   FOnTimeUpdated OnTimeUpdated;

   // 游戏结束的委托，无参数，单纯通知游戏结束
   DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameEnd);
   UPROPERTY(BlueprintAssignable)
   FOnGameEnd OnGameEnd;
   ```

2. 事件触发

   当时间发生变化时，GameState 会触发相应的事件：

   ```cpp
   void AMyGameState::SetRemainingGameTime(float NewTime)
   {
       if (HasAuthority())
       {
           RemainingGameTime = NewTime;
           // 广播时间更新事件
           OnTimeUpdated.Broadcast(RemainingGameTime);
       }
   }

   // 当RemainingGameTime在网络复制时会调用这个函数
   void AMyGameState::OnRep_RemainingTime() const
   {
       // 在客户端接收到新的时间值后也广播事件
       OnTimeUpdated.Broadcast(RemainingGameTime);
   }
   ```

3. 事件响应：以 PlayerController 类为例

   ```cpp
   void AMyPlayerController::BeginPlay()
   {
       Super::BeginPlay();

       // 只在客户端创建UI
       if (IsLocalController())
       {
           if (GameHUDClass != nullptr)
           {
               GameHUDWidget = CreateWidget<UUserWidget>(this, GameHUDClass);
               if (GameHUDWidget != nullptr)
               {
                   GameHUDWidget->AddToViewport();
               }
           }
       }

       // 绑定游戏结束事件, 当游戏结束被通知时，在这里处理相关的逻辑（见总体流程）
       if (AMyGameState* MyGameState = GetWorld()->GetGameState<AMyGameState>())
       {
           MyGameState->OnGameEnd.AddDynamic(this, &AMyPlayerController::OnGameEnd);
       }
   }
   ```

4. 总体流程：

   ```plain text
   服务器端                           网络传输                       客户端
      |                                                               |
   GameMode                                                           |
      |                                                               |
      |---> UpdateGameTime()                                          |
      |          |                                                    |
      |          v                                                    |
      |     检查时间是否>0                                             |
      |          |                                                    |
      |          v                                                    |
      |     GameState                                                 |
      |          |                                                    |
      |          |---> SetRemainingGameTime()                         |
      |          |         |                                          |
      |          |         v                                          |
      |          |     更新RemainingGameTime    ---> 网络复制 ---> GameState
      |          |         |                                          |
      |          |         v                                          |
      |          |     OnTimeUpdated.Broadcast() [服务端]             |---> OnRep_RemainingTime()
      |          |                                                    |         |
      |          |                                                    |         v
      |          |                                                    | OnTimeUpdated.Broadcast() [客户端]
      |          |                                                    |         |
      |          |                                                    |         v
      |          |                                                    |     所有绑定的UI更新
      |          |                                                    |
      |     如果时间 <= 0                                             |
      |          |                                                    |
      |          v                                                    |
      |     GameState.MulticastEndGame() -----> [网络多播RPC] ----> 所有客户端
                                                                      |
                                                                      v
                                                               PlayerController
                                                                      |
                                                                      v
                                                               OnGameEnd()处理
                                                                      |
                                                                      v
                                                               1. 移除游戏HUD
                                                               2. 创建结束界面
                                                               3. 禁用玩家输入
                                                               4. 显示鼠标光标
   ```

# UI 系统

游戏实现了两个简单的 UI（使用蓝图），分别为游戏运行时的 HUD 和游戏结束时的 UI（具体表现参见视频）。

## UI 的管理

UI 的管理主要集中在 MyPlayerController 当中：

1. UI 类的声明

   ```cpp
   // UI类的指针，需要在编辑器中指定对应的蓝图类
   UPROPERTY(EditDefaultsOnly, Category="UI")
   TSubclassOf<class UUserWidget> GameHUDClass;

   UPROPERTY(EditDefaultsOnly, Category="UI")
   TSubclassOf<class UUserWidget> GameOverUIClass;

   // UI实例的指针
   UPROPERTY()
   UUserWidget* GameHUDWidget;

   UPROPERTY()
   UUserWidget* GameOverUIWidget;
   ```

2. UI 的创建与显示

   ```cpp
   void AMyPlayerController::BeginPlay()
   {
       // 只在客户端上创建UI
       if (IsLocalController())
       {
           if (GameHUDClass != nullptr)
           {
               GameHUDWidget = CreateWidget<UUserWidget>(this, GameHUDClass);
               if (GameHUDWidget != nullptr)
               {
                   // 游戏开始时显示运行时的HUD
                   GameHUDWidget->AddToViewport();
               }
           }
       }

       // 绑定游戏结束事件，用来切换UI
       if (AMyGameState* MyGameState = GetWorld()->GetGameState<AMyGameState>())
       {
           MyGameState->OnGameEnd.AddDynamic(this, &AMyPlayerController::OnGameEnd);
       }
   }
   ```

3. 游戏结束时切换 UI

   ```cpp
   void AMyPlayerController::OnGameEnd()
   {
       if (!IsLocalController())
       {
           return;
       }

       // 移除HUD
       if (GameHUDWidget != nullptr)
       {
           GameHUDWidget->RemoveFromParent();
       }

       // 创建并显示游戏结束UI
       if (GameOverUIClass != nullptr)
       {
           GameOverUIWidget = CreateWidget<UUserWidget>(this, GameOverUIClass);
           if (GameOverUIWidget != nullptr)
           {
               DisableInput(this);              // 禁用输入
               bShowMouseCursor = true;         // 显示鼠标
               SetInputMode(FInputModeUIOnly()); // 设置为仅UI输入模式
               GameOverUIWidget->AddToViewport();
           }
       }
   }
   ```

为什么使用 PlayerController 而不是 HUD 来管理 UI？

AHUD 是一个专门用于处理平视显示器（Heads-Up Display）的类，它继承自 AActor。在早期的 UE 版本中，AHUD 主要用于在屏幕上直接绘制内容，如使用 DrawText()、DrawTexture()等方法进行 2D 渲染。

与 PlayerController 中管理 UI 相比，这两种方式各有优势：

**使用 AHUD 的优势：**

1. 专门的职责划分 - AHUD 类的设计初衷就是处理游戏的 UI 展示
2. 直接的 2D 绘制功能 - 对于需要直接在屏幕上绘制的内容（如准星、简单文字提示）非常方便
3. 性能优势 - 对于简单的 UI 元素，直接绘制比创建 Widget 可能更高效

**使用 PlayerController 管理 UMG 的优势：**

1. 更现代的 UI 解决方案 - UMG（Unreal Motion Graphics）提供了更强大的 UI 设计功能
2. 更好的设计工具支持 - 可以在编辑器中可视化设计 UI
3. 更容易实现复杂的交互 - UMG 提供了丰富的交互组件和动画系统
4. 更好的事件处理能力 - PlayerController 自然地连接了游戏逻辑和 UI 交互

简而言之：复杂的用 PlayerController + UMG，简单的用 HUD

## 具体 UI 的设计

### WBP_GameHUD

1. 界面设计

   ![image.png](Image/image.png)

2. 数据更新逻辑

   在 MyGameState 中，定义了两个关键事件：

   ```cpp
   // 游戏状态更新事件
   DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeUpdated, float, NewTime);
   DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpecialCubeUpdated, int, NewSpecialCube);
   ```

   这里的`FOnTimeUpdated`和`FOnSpecialCubeUpdated`用于更新此处剩余时间和剩余特殊方块的值

   大致流程为：获得 GameState 的引用 → 分别绑定`FOnTimeUpdated`和`FOnSpecialCubeUpdated`

   ![image.png](Image/image%201.png)

### WBP_GameOverUI

1. 组件层级：

   ![image.png](Image/image%204.png)

   ```plain text
   GameOverUI（主界面）
       ├── 标题文本（"游戏结束"）
       ├── ScrollBox_Scoreboard（玩家分数列表容器）
       │   └── VerticalBox_ScoreList（单个玩家分数条目）
   	 │        ├── WBP_ScoreEntry
   	 │        │   ├── TextBlock_PlayerID（玩家ID）
   	 │        │   └── TextBlock_PlayerScore（玩家分数）
   	 │			 └── TextBlock_TotalScore（总分）
       └── 退出游戏按钮
   ```

2. 数据更新

   ![image.png](Image/image%205.png)

   ![image.png](Image/image%206.png)
