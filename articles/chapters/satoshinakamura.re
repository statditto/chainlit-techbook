
= Chainlitの基本機能を概観する

//lead{
シンプルなウェブアプリケーションと豊富な図を用いて、Chainlitの機能を解説します。
//}

== 本章で利用するアプリケーション

本章では、ルールベースで応答するシンプルなウェブアプリケーション @<fn>{support} を例に、Chainlitの機能を説明します。アプリケーションの構成は @<img>{app_overview} の通りです。ユーザーはブラウザからアクセスしてチャットを行います。チャットの内容は PostgreSQL データベースに保存されます。

//footnote[support][本章のソースコードは以下から参照できます。@<href>{https://github.com/kimajin/chainlit-playground}]

//image[app_overview][アプリケーション構成図][scale=0.6]{
//}

== ログイン

ユーザーが Chainlit アプリケーションにアクセスすると、ログイン画面が表示されます（@<img>{login}）。
今回は、@<code>{@cl.password_auth_callback} デコレーターを修飾したコールバック関数を定義して、パスワード認証を行っています。

//image[login][ログイン画面][scale=0.6]{
//}

Chainlitの一部の機能は、この認証機能があることではじめて有効となります。例えば、以下の機能が挙げられます。

 * チャット履歴の永続化（ @<code>{@cl.data_layer} ）
 * チャットの再開（ @<code>{@cl.on_chat_resume} ）

Chainlitはその他の認証方法もサポートしています。詳細は Chainlit のドキュメントを参照してください。

 * @<href>{https://docs.chainlit.io/authentication/overview}

認証に成功したら @<code>{cl.User} オブジェクトが返され、チャット開始画面へと遷移します（認証に失敗したら @<code>{None} を返します）。

== チャット

//image[chat_start][チャット開始画面][scale=0.6]{
//}

ログイン後のチャット開始画面は @<img>{chat_start} のようになっています。
メッセージ入力欄以外に、アシスタント選択（画像上部）やスターター（画面中央下部）、過去のチャット（画像左）などのUI要素があります。また入力欄直下にも歯車や猫のアイコンも存在しています。

このセクションでは、チャット画面のこれらのUI要素に関する、基本的な機能と実装方法について説明します。

=== スターター設定（ @<code>{@cl.set_starters} ）

スターターは、ユーザーがチャットを開始する際に選択できるプリセットのメッセージです。スターターをクリックすると対応するメッセージが送信され、チャットが開始されます。

スターターを設定するには、@<code>{@cl.set_starters} デコレーターを使用して、スターターのリストを定義します。個別のスターターは、@<code>{cl.Starter} クラスのインスタンスとして定義します。

//emlist[Starters][python]{
@cl.set_starters
async def set_starters() -> list[cl.Starter]:
    return [
        cl.Starter(
            label="Message",        # 表示されるラベル
            message="Hello World!", # 送信されるメッセージ
        ),
        ...
    ]
//}

スターターは、ユーザーがチャットを開始する際のガイドとして機能し、特に初めてのユーザーにとって便利です。

=== アシスタント選択（ @<code>{@cl.set_chat_profiles} ）

ChatGPTのモデル切り替えと同様の体験で、アシスタントを選択するUIを提供できます。

//image[chat_profile][Chat Profileの選択][scale=0.6]{
//}


UIの提供は、@<code>{@cl.set_chat_profiles} と @<code>{cl.ChatProfile} を利用して実装します。

//emlist[Chat Profiles][python]{
@cl.set_chat_profile
async def set_chat_profiles() -> list[cl.ChatProfile]:
    return [
        cl.ChatProfile(
            name="Assistant Alice",
            markdown_description="A friendly assistant.",
        ),
        cl.ChatProfile(
            name="Assistant Bob",
            markdown_description="A helpful assistant.",
        ),
    ]
//}

アプリケーション側は、ユーザーが選択したアシスタントを @<code>{cl.user_session.get("chat_profile")} から取得できます。

=== チャット再開（ @<code>{@cl.on_chat_resume} ）

@<img>{chat_start}の画面左には、ChatGPTと同様にユーザーが過去に行ったチャット履歴が表示されており、チャットを再開できます。

この機能を利用するためには、認証機能とデータレイヤーの準備に加え、@<code>{@cl.on_chat_resume} の実装が必要になります。
後者の実装は、以下の数行で完了します。

//emlist[Chat Profiles][python]{
@cl.on_chat_resume
async def on_chat_resume(_: ThreadDict) -> None:
    pass
//}

==== データレイヤー ( @<code>{@cl.data_layer} )

チャットを再開するための当然の前提として、チャット履歴が保存されている必要があります。
Chainlitはデータレイヤーと呼ばれる抽象化層を通して、チャットデータの保存と取得を行います。
アプリケーション本体はデータベースの種類を直接意識せず、「データレイヤー」に対してデータの保存や取得を依頼するため、PostgreSQLや独自のストレージなどのさまざまな保存先を柔軟に利用できます。

Chainlit はデータレイヤーの API が実装済みの @<code>{SQLAlchemyDataLayer} を提供しており、開発者はデータベースとテーブルを用意するだけで、データレイヤーとして使えます。
作成するテーブルは以下のリンク先から確認できます。

 * @<href>{https://docs.chainlit.io/data-layers/sqlalchemy}

特に、@<code>{users}、@<code>{threads}、@<code>{steps}というテーブルに、それぞれユーザー、チャット、チャット内のメッセージを永続化しています。@<fn>{note-on-data-layer}
//footnote[note-on-data-layer][チャットやメッセージは更新と削除が可能であることに対応して、対応するレコードもミュータブルとなっています。特に、イベントをイミュータブルに追加していく構造とはなっていません。]

//image[data_layer][Data Layerの例。 steps テーブルにメッセージが保存されている。][scale=1.0]{
//}

@<code>{SQLAlchemyDataLayer} を使う場合、アプリ側でのデータレイヤーの実装は以下の数行で済みます。

//emlist[Data Layer][python]{
@cl.data_layer
def data_layer() -> SQLAlchemyDataLayer:
    return SQLAlchemyDataLayer(conninfo=os.environ["CHAINLIT_CONNINFO"])
//}

ここで、引数 @<code>{conninfo} には、データベースの接続URIを与えています。

なお、Chainlitの @<code>{BaseDataLayer} を具象化することで、カスタムのデータレイヤーを用意することも可能です。実装が必要な API は以下のリンク先から確認できます。

 * @<href>{https://docs.chainlit.io/api-reference/data-persistence/custom-data-layer}

=== コマンド設定（ @<code>{cl.context.emitter.set_commands} ）

メッセージ入力欄の下にあるボタンは、コマンドと呼ばれる機能です。
ボタンを押すか、@<img>{command_meow_from_input} のように「/」から検索して利用できます。
//image[command_meow_from_input][Commandの利用画面][scale=0.6]{
//}

コマンドを選択したあとに、ユーザーがメッセージを送信することで、対応するコマンドの処理が呼び出されます。

//image[command_wc_output][Word Count コマンドの実行結果][scale=0.6]{
//}

ユーザーが選択したコマンドは、ユーザーの送信した @<code>{cl.Message} の @<code>{command} 属性の文字列から確認でき、アプリケーション側はこの文字列をもとに処理を振り分けます。

//emlist[Command処理の実装][python]{
@cl.on_message
async def on_message(message: cl.Message) -> None:
    if command := message.command:
        match command:
            case "Meow":
                await cl.Message(content="Meow!").send()
            case "Word Count":
                word_count = len(message.content.split())
                await cl.Message(content=f"Word count: {word_count}").send()
        return
    ...
//}

また、コマンドを利用するためには事前にアプリケーション側にコマンドを設定しておくことが必要です。コマンドのメタ情報を @<code>{CommandDict} の辞書として記述し、そのリストを @<code>{cl.context.emitter.set_commands} で送ることで設定します。

//emlist[Commandの設定][python]{
@cl.on_chat_start
async def on_chat_start() -> None:
    ...
    await cl.context.emitter.set_commands(
        [
            {
                "id": "Meow",
                "icon": "cat",
                "description": "Sends a meow message",
                "button": True,     # ボタンを表示するかどうか
                "persistent": True, # 実行後もコマンド選択を維持するか
            },
            {
                "id": "Word Count",
                "icon": "ruler",
                "description": "Counts the number of words in the message",
                "button": True,
                "persistent": True,
            },
        ]
    )
//}

@<code>{icon} の値は Lucide@<fn>{lucide} のアイコン名であり、コマンドのボタンに利用されます。
//footnote[lucide][@<href>{https://lucide.dev/}]


スターターと異なり、ユーザーはコマンドの実行時にメッセージを追加できます。
また、コマンドはチャット開始後も利用できます。そのため Skills@<fn>{skills} を呼び出す入口として利用するといった使い方ができます。
//footnote[skills][
 * @<href>{https://code.claude.com/docs/ja/skills}
 * @<href>{https://agentskills.io/home}
//]

=== チャット設定（ @<code>{cl.ChatSettings} ）

 @<img>{chat_start} を見ると、コマンドボタンの隣に歯車のアイコンがあります。
これは、Chat Settings と呼ばれる機能で、チャットの細やかな設定を可能にします。
典型的には、アシスタントが利用するLLMやそのハイパーパラメータの指定を可能とします。

Chat Settings は @<code>{cl.ChatSettings} を介して設定します。
入力ウィジェットにはトグルボタンやスライダーなど様々な種類が用意されており、用途に即して使い分けることができます。

//emlist[Chat Settings][python]{
@cl.on_chat_start
async def on_chat_start() -> None:
    await cl.ChatSettings(
        [
            Select(
                id="Thinking mode",
                label="Thinking mode",
                values=["fast", "slow"],
                initial_index=0,
            ),
            Slider(
                id="Creativity",
                label="Creativity",
                initial=50,
                min=0,
                max=100,
            ),
            Switch(
                id="Enable feature X",
                label="Enable feature X",
                initial=False,
            ),
            Tags(
                id="Interests",
                label="Interests",
                values=["AI", "Machine Learning", "Data Science"],
                initial=["AI", "Data Science"],
            ),
            TextInput(
                id="Notes",
                label="Notes",
                placeholder="Enter your notes here",
            ),
        ]
    ).send()
    ...
//}

ユーザーが歯車のアイコンを押すと設定画面がポップアップされます。

//image[chat_setting][チャット設定画面][scale=0.8]{
//}

設定が更新されると、@<code>{@cl.on_settings_update} に登録した関数が呼び出されます。
設定をユーザーセッションに保存することで、アプリケーション側が設定内容に応じた処理を行えます。

//emlist[Chat Settings Update][python]{
@cl.on_settings_update
async def setup_agent(settings: dict[str, Any]) -> None:
    cl.user_session.set("chat_settings", settings)
//}

== メッセージ

チャット内では、ユーザーとアシスタントのメッセージのやり取りが行われます。
ユーザーがメッセージを送信すると、@<code>{@cl.on_message} を修飾したコールバック関数が呼び出され、そのなかでアシスタントの処理と返答が行われます。

//image[command_meow_output][チャット画面][scale=0.8]{
//}

しかし、アシスタントの回答といってもその表示方法は様々です。
例えば、ChatGPTを見ると、最終的な回答メッセージだけではなく、途中の思考やツールの実行内容が表示されていることが見て取れます。
また、Claude Code や Codex などのコーディングエージェントを利用すると、エージェントがユーザにYes/Noを提示して許可を求めたり、選択肢を提示して質問を投げかけることがあります。

そのため、アシスタントの意図を表すには、メッセージに関する UI 上の工夫が必要です。
これらの要求に幅広く対応するため、Chainlitでは様々な機能が提供されています。
この節では、ChainlitのメッセージのUI要素について、基本的な機能と実装方法を説明します。

=== Message（ @<code>{cl.Message} ）

@<code>{cl.Message} はユーザとアシスタントがやり取りする最も基本的なメッセージです。
ユーザの入力したメッセージや、アシスタントの出力した回答に対応します。

例えば、ユーザーの入力をそのまま出力する場合、実装は以下のようになります。

//emlist[@<code>{cl.Message}][python]{
@cl.on_message
async def on_message(message: cl.Message) -> None:
    await cl.Message(content=f"Received: {message.content}").send()
//}

特に、ユーザーの入力メッセージは関数の引数を通じて与えられ、メッセージの文字列は @<code>{content} 属性にあり、メッセージの送信は @<code>{send()} で行われています。

@<code>{send()} を呼び出すと、内部では以下のことが行われます。
@<fn>{messagebase-send}
//footnote[messagebase-send][詳細は @<code>{MessageBase.send()} の実装を参照してください。]

 * UI画面へのメッセージ送信（メッセージが表示される）
 * （もしあれば）データレイヤーにメッセージのレコードを追加
 * @<code>{cl.chat_context} にメッセージを追加

最後の @<code>{cl.chat_context} は、チャット内でこれまでやり取りしたメッセージを @<code>{list[cl.Message]} として保存している変数です。
特に、@<code>{cl.chat_context.get()} や @<code>{cl.chat_context.to_openai()} で取得したメッセージの履歴を LLM の入力として与えるといった使い方ができます。

なお、メッセージの内容は @<code>{update()} や @<code>{remove()} を用いて更新・削除できます（@<code>{send()} と同様に、内部で更新・削除が行われます）。

//emlist[ @<code>{update()} 及び @<code>{remove()}][python]{
@cl.on_message
async def on_message(message: cl.Message) -> None:
    msg = cl.Message(content="This message will be updated after 2 seconds.")
    await msg.send()

    await cl.sleep(2)

    msg.content = "This message will be removed after 2 seconds."
    await msg.update() # このタイミングでメッセージが更新される

    await cl.sleep(2)
    await msg.remove() # このタイミングでメッセージが削除される
//}

=== Step（ @<code>{cl.Step} ）

LLM アプリケーションでは、AI の思考の過程をユーザに表示したい場面があります。例えば次のような情報です。

 * 検索ツールを使った
 * データベースを参照した
 * 中間計算を行った

Chainlit では、このような処理過程を可視化するための仕組みとして
@<code>{cl.Step} が用意されています。

使い方を学ぶため、以下の実装と対応する実行結果を見てみましょう。

//emlist[ @<code>{cl.Step}][python]{
@cl.on_message
async def on_message(message: cl.Message) -> None:
    ...
    async with cl.Step(
        name="Step started", # ステップに最初に表示される名前
        default_open=True,
    ) as step:
        await cl.sleep(1)
        ...

        async with cl.Step(         # ステップはネストすることができます。
            name="Tool call",       # 表示されるラベル
            type="tool",            # ステップの種類を表すタグ
            default_open=True,
            show_input="python",    # 入力のシンタックスハイライト
        ) as second:
            step.name = second.name

            second.input = "add(1, 2)"
            await step.update()
            await second.update()
            await cl.sleep(1)
            second.output = {"output": 3}

        step.name = "Step completed"
//}

//image[step][@<code>{cl.Step} の表示][scale=0.8]{
//}

このネストされたステップを実行すると、以下のように途中経過を表示しながら処理が進みます。

 1. 親ステップが @<code>{__aenter__()} し、@<code>{send()} により、「使用中：Step started」という文字列と共にステップが表示される。
 1. 子ステップが @<code>{__aenter__()} し、ネストされた形で同様に表示される。
 1. 子ステップの @<code>{input} と 親ステップの @<code>{name} が更新される。 @<code>{update()} が呼び出されて画面に反映される。
 1. 子ステップの @<code>{output} が更新され、子ステップが @<code>{__aexit__()} する。@<code>{__aexit__()} 内部で @<code>{update()} が呼び出されてUIに反映される。
 1. 親ステップが @<code>{__aexit__()} する。表示は「使用済み：Step completed」となる。

コンテキストマネージャーのため分かりにくいですが、上の処理をみると @<code>{cl.Step} は @<code>{cl.Message} と同様のメソッドを持っていることが分かります。
対応して @<code>{cl.Step} の内容も @<code>{cl.Message} と同様にデータレイヤーの @<code>{steps} テーブルに永続化されます。
一方で、メッセージ履歴を管理する @<code>{cl.chat_context} には反映されません。
ここから、@<code>{cl.Step} が思考の過程を表現する機能であるという意図が読み取れます。

=== Action（ @<code>{cl.Action} ）

@<code>{cl.Action} は、ユーザがクリックできる「操作ボタン」を表すオブジェクトです。
@<code>{cl.Action} をメッセージに追加すると、チャット UI 上にボタンとして表示されます。
ユーザがボタンをクリックすると、アプリケーション側で対応する処理が実行されます。
そのため、@<code>{cl.Action}を再実行や確認ダイアログのトリガーに利用できます。

以下は、クリック回数を表示する単純なアクションです。

//emlist[@<code>{cl.Action}][python]{
@cl.action_callback("count_clicks")
async def count_clicks(action: cl.Action) -> None:
    await action.remove()
    count_key = f"count:{action.forId}"

    count = cl.user_session.get(count_key, 0)
    count += 1
    cl.user_session.set(count_key, count)

    action.payload["count"] = count
    action.label = f"Clicked {count} times!"
    await action.send(for_id=action.forId)

@cl.on_message
async def on_message(message: cl.Message) -> None:
    ...
    actions = [
        cl.Action(
            name="count_clicks",
            payload={"count": 0},
            label="Click me!",
            tooltip="This button will count the number of clicks.",
            icon="mouse-pointer-click",
        )
    ]
    await cl.Message(
        content="Let's click some buttons!", actions=actions
    ).send()
//}

 * @<code>{Action.name} がアクションの識別子となります。
 * @<code>{cl.Message} に複数のアクションを設定できます。
 * アクションのボタンが押された場合のコールバックは @<code>{@cl.action_callback("アクション名")} で実装します。

実行すると以下のようにクリックできるボタンが出現し、クリック回数が表示されます。

//image[action][@<code>{cl.Action} の表示][scale=0.8]{
//}

なお、@<code>{cl.Action}の内容は、データレイヤーの永続化の対象ではありません。
そのため、例えばタブをリロードすると、メッセージのボタンが無くなります。
このことから、アクションの選択内容を後で追跡したい場合は、@<code>{cl.Message} や @<code>{cl.Step} に選択結果を記録するなどの対応が必要となります。
例えば、確認ダイアログに利用する場合には「Yes/No」のボタンだけではなく、アシスタントの回答に「良ければYes、良くないならばNoと回答してください」とすることで、再開時にボタンが消えたとしてもユーザーが迷わないメッセージ設計になります。

=== Element（ @<code>{cl.Element} ）

アシスタントの回答として画像や動画といった、テキスト以外の要素を含めたい場合があります。
Chainlitでは、これらの要素を扱うために @<code>{cl.Element} が用意されています。

使い方は以下のように、@<code>{cl.Message} に付属させる形式となります。

//emlist[@<code>{cl.Element}][python]{
@cl.on_message
async def on_message(message: cl.Message) -> None:
    ...
    elements = [
        cl.Text(
            name="text",
            display="inline",
            content="a text element with inline display.",
        ),
    ]
    await cl.Message(
        content="Here are some elements:", elements=elements
    ).send()
//}

Chainlitの@<code>{cl.Element}は画像や動画など様々な要素に対応しています。
対応範囲については以下を参照してください。

 * @<href>{https://docs.chainlit.io/concepts/element}
 * @<href>{https://docs.chainlit.io/api-reference/elements/custom}

=== Ask User（ @<code>{cl.AskMessageBase} ）

先ほど、 @<code>{cl.Action}を使ってユーザーのボタン操作を促す方法を説明しましたが、
もしユーザーにメッセージ入力を要求をしたい状況では、 @<code>{cl.AskUserMessage} および @<code>{cl.AskMessageBase} を具象化したクラスが有用です。
例えば、以下の例では、アシスタントがユーザーに好きな色を聞いています。

//emlist[ @<code>{cl.AskUserMessage}][python]{
@cl.on_message
async def on_message(message: cl.Message) -> None:
    ...
    res = await cl.AskUserMessage(
        content="What is your favorite color?",
    ).send()
    if res:
        await cl.Message(
            content=f"Your favorite color is {res['output']}"
        ).send()
//}

これを実行すると以下のようになります。

//image[ask][@<code>{cl.AskUserMessage}][scale=0.8]{
//}

実装をみれば分かるように、ユーザーからの回答を待っている間、@<code>{@cl.on_message}のコールバックは処理を停止している状況となります。
ユーザーが離脱したときに待機し続けてしまうことを避けるため、@<code>{cl.AskUserMessage} にはタイムアウト時間を設定できます。デフォルトでは60秒後に自動的にタイムアウトします。

== まとめ

本章では、Chainlitの基本的な機能を、シンプルなウェブアプリケーションを例に概観しました。

まず、ログイン機能では、@<code>{@cl.password_auth_callback} を用いたパスワード認証を実装し、これによりアシスタント選択やチャット履歴の永続化などの機能が有効になることを説明しました。

次に、チャット機能として、スターター（@<code>{@cl.set_starters}）によるプリセットメッセージ、アシスタント選択（@<code>{@cl.set_chat_profiles}）、チャット再開（@<code>{@cl.on_chat_resume}）とデータレイヤー（@<code>{@cl.data_layer}）による履歴管理、コマンド設定（@<code>{cl.context.emitter.set_commands}）、チャット設定（@<code>{cl.ChatSettings}）を紹介しました。これらの機能により、ユーザビリティの高いチャットインターフェースを実現できます。

さらに、メッセージ機能では、基本的なメッセージ（@<code>{cl.Message}）、処理過程の可視化（@<code>{cl.Step}）、ユーザー操作のボタン（@<code>{cl.Action}）、テキスト以外の要素（@<code>{cl.Element}）、ユーザー入力の要求（@<code>{cl.AskUserMessage}）を説明しました。これにより、アシスタントの応答を多様な形で表現可能です。

Chainlitは、これらの機能を組み合わせることで、LLMを活用したインタラクティブなアプリケーションを効率的に開発できるフレームワークです。開発者は、認証、UI要素、データ管理の詳細を意識せずに、ビジネスロジックに集中することができます。
